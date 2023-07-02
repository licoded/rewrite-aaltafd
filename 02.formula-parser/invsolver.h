/**
 * File:   invsolver.h
 * Author: Yongkang Li
 *
 * Created on July 02, 2023, 10:13 AM
 */

#ifndef INV_SOLVER_H
#define INV_SOLVER_H 

#include "aaltasolver.h"
#include <vector>

namespace aalta
{
    class InvSolver : public AaltaSolver
    {
    public:
        InvSolver(int id, bool verbose = false) : AaltaSolver(verbose), flag_id_(id) {}
        // functions
        void create_flag_for_frame(int frame_level);
        void add_clauses_for_frame(std::vector<int> &uc, int frame_level);
        bool solve_with_assumption(int frame_level);
        inline int new_var() { return ++flag_id_; }
        void update_assumption_for_constraint(int id);
        void disable_frame_and();

    protected:
        // ids in SAT solver to represent each frame, i.e. frame_flags[i] represents frame i;
        std::vector<int> frame_flags_;
        // the flag id to represent the flags of each frame
        int flag_id_;
    };
}

#endif