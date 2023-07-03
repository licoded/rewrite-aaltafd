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

        int frame_level = 0;
        // initialize the first frame
        // tmp_frame_.clear(); // don't need this, as the default initial frame_ is just empty
        add_frame_element(frame_level);
        add_new_frame();

        while (true)
        {
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
        // add/copy/sync a new frame to SAT solver (CARSolver)
        int frame_level = frames_.size() - 1;
        carsolver_->create_flag_for_frame(frame_level);
        for (int i = 0; i < tmp_frame_.size(); i++) // tmp_frame_ is a set of multiple uc(s) -- a two-dimensional array
            carsolver_->add_clause_for_frame(tmp_frame_[i], frame_level); // tmp_frame_[i] is a uc -- a one-dimensional array
        tmp_frame_.clear();
    }

    /**
     * try to find a model with the length of @frame_level
     * NOTE: we always starts with the input formula f at each beginning of our recursion !!!
     *       the difference is the expected length of the SAT path to the final/close state
    */
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
                    add_frame_element(frame_level);
            }
            if (try_satisfy(t->next(), frame_level - 1))
                return true;
        }
        add_frame_element(frame_level + 1);
        return false;
    }

    /**
     * add `uc=get_uc()` into specified frame, and update corresponding frame in CARSolver if needed
     * when update is needed: the frame, which uc is added into, has been pushed into frames_ and generated in CARSolver
    */
    void CARChecker::add_frame_element(int frame_level)
    {
        std::vector<int> uc = carsolver_->get_selected_uc(); // has invoked sat_once(f) before, so uc has been generated
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
     * check whether /\ (1<=j<=i) C[j] -> C[i] is true.
     * ATTENTION: inv_found = !solve_assumption(), so solve_assumption() should check SAT of !( /\ (1<=j<=i) C[j] -> C[i] )
     *                  ( PROOF: !(a->b) = !(!a \/ b) = a /\ !b )                       ====    /\ (1<=j<=i) C[j] /\ !C[i]
     * ATTENTION: Let φ be a formula of propositional logic. Then φ is satisﬁable iff ¬φ is not valid.
     *            So in order to prove φ is not valid, we just need to prove ¬φ is SAT. !!!
     * ATTENTION: the clauses added by `add_clause()` are /\ not \\/ !!!
    */
    bool CARChecker::solve_inv_at(int frame_level)
    {
        // add_clause -- !C[i] ==== ! \/ uc[i]
        inv_solver_->add_clauses_for_frame_and(frames_[frame_level]);
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