/**
 * File:   carchecker.h
 * Author: Yongkang Li
 *
 * Created on July 02, 2023, 10:13 AM
 */

#ifndef CAR_CHECKER_H
#define CAR_CHECKER_H

#include "ltlfchecker.h"
#include "carsolver.h"
#include "invsolver.h"
#include "formula/aalta_formula.h"
#include <vector>

namespace aalta
{
    class CARChecker
    {
    public:
        CARChecker(aalta_formula *f, bool verbose = false) : to_check_(f) {}
        ~CARChecker() {}

        bool check();

    private:
        // members
        aalta_formula *to_check_;
        typedef std::vector<std::vector<int>> Frame;
        std::vector<Frame> frames_; // frame sequence
        Frame tmp_frame_;           // temporal frame to store the UCs before it is pushed into frames_
        CARSolver *carsolver_;
        InvSolver *inv_solver_;     // SAT solver to check invariant

        // functions
        // main checking function
        bool car_check(aalta_formula *f);
        // try to find a model with the length of \@frame_level
        bool try_satisfy(aalta_formula *f, int frame_level);
        // add \@uc to frame \@frame_level
        void add_frame_element(int frame_level, std::vector<int> &uc);
        // check whether an invariant can be found in up to \@frame_level steps.
        bool inv_found(int frame_level);
        // add a new frame to frames_
        void add_new_frame();
        // add a new frame to SAT solver
        void solver_add_new_frame();
        // check whether an invariant is found at frame \@ i
        bool inv_found_at(int i);

        // check whether \@ f can be a final state
        bool sat_once(aalta_formula *f);

        // handle inv_solver_
        bool solve_inv_at(int frame_level);
        void add_clauses_to_inv_solver(int level);
        void add_clauses_to_inv_solver_for_frame_or(Frame &frame);
        void add_clauses_to_inv_solver_for_frame_and(Frame &frame);
    };
}

#endif