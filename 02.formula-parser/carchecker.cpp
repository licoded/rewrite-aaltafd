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
            return false;                // 这里可以判定 false, 是因为之前有 sat_once(f) is false
        }

        // initialize the first frame
        std::vector<int> uc = carsolver_->get_selected_uc();    // has invoked sat_once(f) before, so uc has been generated
        tmp_frame_.push_back(uc);
        add_new_frame();

        int frame_level = 0;
        while (true)
        {
            tmp_frame_.clear();
            if (try_satisfy(f, frame_level))
                return true;
            if (inv_found(frame_level))
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
            carsolver_->add_clause_for_frame(tmp_frame_[i], frame_level);   // tmp_frame_[i] is uc
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
        {
            tmp_frame_.push_back(uc);
        }
        else
        {
            frames_[frame_level].push_back(uc);
            carsolver_->add_clause_for_frame(uc, frame_level);
        }
    }

    bool CARChecker::inv_found(int frame_level)
    {
        bool res = false;
        inv_solver_ = new InvSolver(to_check_->id());
        for (int i = 0; i < frames_.size(); i++)
        {
            if (inv_found_at(i))
            {
                res = true;
                break;
            }
        }
        delete inv_solver_;
        return res;
    }

    bool CARChecker::inv_found_at(int frame_level)
    {
        if (frame_level == 0)
        {
            add_clauses_to_inv_solver_for_frame_or(frames_[frame_level]);
            return false;
        }
        return solve_inv_at(frame_level);
    }

    bool CARChecker::solve_inv_at(int frame_level)
    {
        add_clauses_to_inv_solver_for_frame_and(frames_[frame_level]);
        bool res = !(inv_solver_->solve_assumption());
        inv_solver_->disable_frame_and();
        add_clauses_to_inv_solver_for_frame_or(frames_[frame_level]);
        return res;
    }

    void CARChecker::add_clauses_to_inv_solver_for_frame_or(Frame &frame)
    {
        std::vector<int> v;
        for (int i = 0; i < frame.size(); i++)
        {
            int clause_flag = inv_solver_->new_var();
            v.push_back(clause_flag);
            for (int j = 0; j < frame[i].size(); j++)
                inv_solver_->add_clause(-clause_flag, frame[i][j]);
        }
        inv_solver_->add_clause(v);
    }

    void CARChecker::add_clauses_to_inv_solver_for_frame_and(Frame &frame)
    {
        int frame_flag = inv_solver_->new_var();
        for (int i = 0; i < frame.size(); i++)
        {
            std::vector<int> v;
            for (int j = 0; j < frame[i].size(); j++)
                v.push_back(-frame[i][j]);
            v.push_back(-frame_flag);
            inv_solver_->add_clause(v);
        }
        inv_solver_->update_assumption_for_constraint(frame_flag);
    }

    bool CARChecker::sat_once(aalta_formula *f)
    {
        return carsolver_->check_final(f);
    }
}