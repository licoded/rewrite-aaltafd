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
        // create a new var; occupy a flag_id_
        inline int new_var() { return ++flag_id_; }
        void update_assumption_for_constraint(int id);
        void disable_frame_and();

    protected:
        // ids in SAT solver to represent each frame, i.e. frame_flags[i] represents frame i;
        std::vector<int> frame_flags_;
        // the flag id to represent the flags of each frame
        int flag_id_;
        /**
         * TODO: ABOUT flag_id_, it's very strange
         *          - Why this is not static? Because this class will only have one instance?
         *              - I think this is true! And we should make a Singleton Pattern in the future. (TODO)
         *          - flag_id_ is initialized with the (what) id of the formula to be checked, and then it will increase in InvSolver. !!!
         *              - (what) id: id in SAT solver
         * ALREADY KNOW:
         *          - the meaning of the word 'flag(s)' is equivalent to 'represent'.
        */
    };
}

#endif