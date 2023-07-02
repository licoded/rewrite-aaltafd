/**
 * File:   invsolver.cpp
 * Author: Yongkang Li
 *
 * Created on July 02, 2023, 13:56 PM
 */

#include "invsolver.h"
#include <vector>

namespace aalta
{
    void InvSolver::create_flag_for_frame(int frame_level)
    {
        assert(frame_flags_.size() == frame_level);
        frame_flags_.push_back(++flag_id_);
    }

    void InvSolver::add_clauses_for_frame(std::vector<int> &uc, int frame_level)
    {
        assert(frame_level < frame_flags_.size());
        std::vector<int> v;
        // if v is (a1, a2, ..., an), create clause (flag, -a1, -a2, ..., -an)
        v.push_back(frame_flags_[frame_level]);
        for (int i = 0; i < uc.size(); i++)
            v.push_back(-uc[i]);
        add_clause(v);

        // create clauses (-flag, a1),(-flag, a2) ... (-flag, an)
        v.clear(); // TODO: why need to clear this temporary variable?
        for (int i = 0; i < uc.size(); i++)
            add_clause(-frame_flags_[frame_level], uc[i]); // frame_flags_[frame_level] -> uc[i], NOTE: it's just single direction, not a equivalence (double direction)
    }

    // add all elems of `frame_flags` into `assumption_`, the last one (idx=frame_level) is nagative!
    bool InvSolver::solve_with_assumption(int frame_level)
    {
        assumption_.clear();
        for (int i = 0; i < frame_level; i++)
            assumption_.push(id_to_lit(frame_flags_[i]));
        assumption_.push(id_to_lit(-frame_flags_[frame_level]));
        return solve_assumption();
    }

    // add LitId represented by id into `assumption_`
    void InvSolver::update_assumption_for_constraint(int id)
    {
        assumption_.push(id_to_lit(id));
    }

    // change last level element in `assumption_` from `l` to `l^1`, represent `x` to `-x`
    void InvSolver::disable_frame_and()
    {
        Minisat::Lit l = assumption_.last();
        assumption_.pop();
        assumption_.push(~l);
    }
}