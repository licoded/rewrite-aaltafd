/**
 * File:   carsolver.h
 * Author: Yongkang Li
 *
 * Created on June 26, 2023, 16:28 AM
 */

#ifndef CAR_SOLVER_H
#define CAR_SOLVER_H

#include "solver.h"
#include "formula/aalta_formula.h"
#include <unordered_set>

namespace aalta
{
	class CARSolver : public Solver
	{
	public:
		CARSolver(aalta_formula *f, bool verbose = false, bool partial_on = false, bool uc_on = true) : Solver(f, verbose, partial_on, uc_on){};

		// functions
		bool solve_with_assumption(aalta_formula *f, int frame_level);
		void add_clause_for_frame(std::vector<int> &uc, int frame_level);
		void create_flag_for_frame(int frame_level);

		std::vector<int> get_selected_uc();
		bool check_final(aalta_formula *f);

	protected:
		// members
		// ids to flag each frame, i.e. frame_flags[i] represent the id for frames_[i]
		std::vector<int> frame_flags_;
		unordered_set<int> selected_assumption_; // get the UC from elements stored in this set

		void set_selected_assumption(aalta_formula *f);
	};
}

#endif
