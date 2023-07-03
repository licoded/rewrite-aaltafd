/**
 * File:   carchecker.h
 * Author: Yongkang Li
 *
 * Created on July 02, 2023, 10:13 AM
 */

#include "carchecker.h"
#include <iostream>
using namespace std;
using namespace Minisat;

namespace aalta
{
    bool CARChecker::check()
    {
        if (to_check_->oper() == e_true)
            return true;
        if (to_check_->oper() == e_false)
            return false;
        return car_check(to_check_);
    }

    bool CARChecker::car_check(aalta_formula *f)
    {
        if (sat_once(f))
            return true;
        if (f->is_wider_globally())
        {
            carsolver_->block_formula(f); // 同样也是因为有 sat_once(f) is false
            return false;                 // 这里可以判定 false, 是因为之前有 sat_once(f) is false
        }

        // initialize the first frame
        std::vector<int> uc = carsolver_->get_selected_uc(); // has invoked sat_once(f) before, so uc has been generated
        tmp_frame_.push_back(uc);
        add_new_frame();

        int frame_level = 0;
        while (true)
        {
            tmp_frame_.clear();
            if (try_satisfy(f, frame_level))
                return true;
            if (inv_found())
                return false;
            add_new_frame();
            frame_level++;
        }
        return false;
    }

    void CARChecker::add_new_frame()
    {
        frames_.push_back(tmp_frame_);
        solver_add_new_frame();
    }

    void CARChecker::solver_add_new_frame()
    {
        int frame_level = frames_.size() - 1;
        carsolver_->create_flag_for_frame(frame_level);
        for (int i = 0; i < tmp_frame_.size(); i++)
            carsolver_->add_clause_for_frame(tmp_frame_[i], frame_level); // tmp_frame_[i] is uc
    }

    bool CARChecker::try_satisfy(aalta_formula *f, int frame_level)
    {
        // check whether \@f has a next state that can block constraints at level \@frame_level
        while (carsolver_->solve_with_assumption(f, frame_level))
        {
            Transition *t = carsolver_->get_transition();
            if (frame_level == 0)
            {
                if (sat_once(t->next()))
                    return true;
                else
                {
                    std::vector<int> uc = carsolver_->get_selected_uc();
                    add_frame_element(frame_level, uc);
                    continue;
                }
            }
            if (try_satisfy(t->next(), frame_level - 1))
                return true;
        }
        std::vector<int> uc = carsolver_->get_selected_uc();
        add_frame_element(frame_level + 1, uc);
        return false;
    }

    void CARChecker::add_frame_element(int frame_level, std::vector<int> &uc)
    {
        assert(!uc.empty());
        if (frame_level == frames_.size())
            tmp_frame_.push_back(uc);
        else
        {
            frames_[frame_level].push_back(uc);
            carsolver_->add_clause_for_frame(uc, frame_level);
        }
    }

    // check whether an invariant can be found in up to \@frame_level steps.
    // if `return false`, means UNSAT
    bool CARChecker::inv_found()
    {
        bool res = false;
        inv_solver_ = new InvSolver(to_check_->id());
        int cur_frame_level = 0;
        while (cur_frame_level < frames_.size() && !res)
            res = inv_found_at(cur_frame_level);
        delete inv_solver_;
        return res;
    }

    /**
     * Algorithm after reduction: whether /\ (1<=j<=i) C[j] -> C[i] is SAT
     * NOTE: Attention that the codes may not correspond to the paper, may not do the reduction!
     *          - I think it indeedly do as the paper said, because the `solve_inv_at()` func has param frame_level.
     * NOTE: the type of C[i][j] in C[i] (type: Frame is two-dimensional array) is vector<int> is one-dimensional array
    */
    bool CARChecker::inv_found_at(int frame_level)
    {
        if (frame_level == 0)
        {
            // add_clause -- C[i] ==== \/ uc[i]
            inv_solver_->add_clauses_for_frame_or(frames_[frame_level]);
            return false; // because if i==1, we should return false, as it doesn't have previous level/frame
        }
        return solve_inv_at(frame_level);
    }

    /**
     * ATTENTION: inv_found = !solve_assumption(), so solve_assumption() should check !( /\ (1<=j<=i) C[j] -> C[i] )
     *               ( PROOF: !(a->b) = !(!a \/ b) = a /\ !b )                   ====    /\ (1<=j<=i) C[j] /\ !C[i]
     * NOTE: the clauses added by `add_clause()` are /\ not \\/ !!!
    */
    bool CARChecker::solve_inv_at(int frame_level)
    {
        // add_clause -- !C[i] ==== ! \/ uc[i]
        inv_solver_->add_clauses_for_frame_and(frames_[frame_level]);
        /**
         * add_clause(a1); add_clause(a2);
         * After the codes of above line, the relation is a1 /\ a2
         * 
         * SO, the `solve_assumption()` in following line check, !( /\ (1<=j<=i) C[j] -> C[i] )
        */
        bool inv_found = !(inv_solver_->solve_assumption());
        /**
         * just disable/remove the previous !C[i]
         * NOTE: why not just remove it from assumptions_?
         *       - because it is more clean and efficient
         *         to specify `!a` to be true to remove `a` and `a->b`
         *         as `!a` make `a->b` always be true.
        */
        inv_solver_->disable_frame_and();
        // add_clause -- C[i] ==== \/ uc[i]
        inv_solver_->add_clauses_for_frame_or(frames_[frame_level]);
        return inv_found;
    }

    bool CARChecker::sat_once(aalta_formula *f)
    {
        return carsolver_->check_final(f);
    }
}