/**
 * File:   ltlfchecker.cpp
 * Author: Yongkang Li
 *
 * Created on June 26, 2023, 16:26 AM
 */

#include "ltlfchecker.h"

namespace aalta
{
	bool LTLfChecker::check()
	{
		return dfs_check(to_check_);
	}

	/**
	 * The core func!
	 *
	 * NOTE: No matter which loop and which recursion, the SAT solver is the same one.
	 * 		 It means that the `add_clause()` will accumulate all the time.
	 */
	bool LTLfChecker::dfs_check(aalta_formula *f)
	{
		if (detect_unsat())
			return false;
		if (sat_once(f))
			return true;

		// TODO: block `af *f` to avoid dead loop?
		//		 f here represents a set of automata?
		push_formula_to_explored(f);

		if (f->is_wider_globally())
			// As above result of `sat_once(f)` is already false, and f is G formula, so f cannot be SAT !!!
			return false;

		// TODO: Add the following heuristics codes back, when/after the `dfs_check()` is tested work successfully.
		// heuristics: if the global parts of f is unsat, then f is unsat
		/*
		if (contain_global(f))
		{
			if (global_part_unsat(f))
			{
				push_uc_to_explored();
				return false;
			}
		}
		*/

		while (true)
		{
			if (detect_unsat())
				return false;
			/**
			 * Why just check `detect_unsat()`, why not check `sat_once()` here.
			 * 	- doing `detect_unsat()` is to exit loop if UNSAT
			 * 		- if uc is empty (when Tail /\ xnf(\phi) is UNSAT),
			 * 		- 	we will conclude the formula f to be checked is UNSAT
			 */
			Transition *t = get_one_transition_from(f);
			if (t != NULL) // Tail /\ xnf(\phi) is SAT
			{
				if (dfs_check(t->next()))
				{
					delete t;
					return true;
				}
			}
			else // UNSAT, cannot get new states, that means f is not used anymore
			{
				push_uc_to_explored(); // we will conclude the formula f to be checked is UNSAT if uc is empty
				delete t;
				return false;
			}
		}
		return false;
	}

	Transition *LTLfChecker::get_one_transition_from(aalta_formula *f)
	{
		if (solver_->solve_by_assumption(f))
			return solver_->get_transition();
		return NULL;
	}

	/**
	 * Just block f in SAT solver.
	 */
	void LTLfChecker::push_formula_to_explored(aalta_formula *f)
	{
		solver_->block_formula(f);
	}

	/**
	 * Just get uc and block it in SAT solver. (premise: uc_on_ is true)
	 */
	void LTLfChecker::push_uc_to_explored()
	{
		solver_->block_uc();
	}

	/**
	 * NOTE: sat_once() test, if current state is TAIL (ending state), whether f can be SAT.
	*/
	bool LTLfChecker::sat_once(aalta_formula *f)
	{
		return solver_->check_tail(f);
	}
}