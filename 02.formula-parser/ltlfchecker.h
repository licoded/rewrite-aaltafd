/**
 * File:   ltlfchecker.h
 * Author: Yongkang Li
 *
 * Created on June 26, 2023, 16:26 AM
 */

#ifndef LTLF_CHECKER_H
#define LTLF_CHECKER_H

#include "formula/aalta_formula.h"
#include "carsolver.h"

namespace aalta
{
	class LTLfChecker
	{
	public:
		LTLfChecker(){};
		LTLfChecker(aalta_formula *f, bool verbose = false) : to_check_(f), verbose_(verbose)
		{
			solver_ = new CARSolver(f, verbose);
		}
		void create_solver() {}
		~LTLfChecker()
		{
			if (solver_ != NULL)
				delete solver_;
		}
		bool check();

	protected:
		// flags
		bool verbose_;		// default is false
		CARSolver *solver_; // SAT solver for computing next states
		aalta_formula *to_check_; // used in ctor
		std::vector<aalta_formula *> visited_; // visited_ is updated during the search process.

		//////////functions
		bool sat_once(aalta_formula *f); // check whether the formula can be satisfied in one step (the terminating condition of checking)
		bool contain_global(aalta_formula *); // may add back in dfs_check(), as we now throw away check_with_heuristics()
		bool global_part_unsat(aalta_formula *); // may add back in dfs_check(), as we now throw away check_with_heuristics()
		bool dfs_check(aalta_formula *f);
		Transition* get_one_transition_from(aalta_formula *);
		void push_formula_to_explored(aalta_formula *f);
		void push_uc_to_explored();
		void print_formulas_id(aalta_formula *);
		inline bool detect_unsat() { return solver_->unsat_forever(); }
	};

}

#endif
