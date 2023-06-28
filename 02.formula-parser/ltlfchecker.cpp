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

	bool LTLfChecker::dfs_check(aalta_formula *f)
	{
		/**
		 * NOTE: No matter which loop and which recursion, the SAT solver is the same one.
		 * 		 It means that the `add_clause()` will accumulate all the time.
		 */
		visited_.push_back(f);
		/**
		 * NOTE: All cases will `pop_back(f)` after execution of current func, except `return true;`
		 *
		 * TODO:
		 * 		- why not just `return false;`?
		 * 		- how can TRUE cases not `pop_back(f)`?
		 */

		if (detect_unsat())
			return false;
		if (sat_once(f))
			return true;
		if (f->is_globally())
		{
			/**
			 * My understanding:
			 * 		As `sat_once(f)` is false, and f is G formula, so f cannot be SAT.
			 * 		- TODO: sat_once() test, if current state is TAIL (ending state), whether f can be SAT.
			 */
			visited_.pop_back();
			push_formula_to_explored(f);
			return false;
		}

		// TODO: add the following heuristics codes back!
		// heuristics: if the global parts of f is unsat, then f is unsat
		/*
		if(contain_global (f))
		{
			if (global_part_unsat (f))
			{
				if (verbose_)
					cout << "Global parts are unsat" << endl;
				visited_.pop_back ();
				push_uc_to_explored ();
				return false;
			}
		}
		*/

		/**
		 * OLD COMMENTS: The SAT solver cannot return f as well
		 *
		 * TODO: Why?
		 * 			- may in order to avoid dead loop?
		 * 				- So, it is bind with `visited_.pop_back()`
		 * 					- No, it is about `add_cluase()`, not `visited_`
		 * 				- But, it still in order to avoid dead loop, I think.
		 */
		push_formula_to_explored(f);

		while (true)
		{
			if (detect_unsat())
				return false;
			/**
			 * TODO: Why not check `sat_once()` here.
			 * 			- I think it is because the below `get_one_transition_from(f)` has already do this.
			 * 			- But the new problem is why `detect_unsat()` here?
			 * 				- I think `detect_unsat()` is also included in `get_one_transition_from(f)`, so why?
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
				visited_.pop_back();
				push_uc_to_explored();
				delete t;
				return false;
			}
		}
		visited_.pop_back();
		return false;
	}

	Transition *LTLfChecker::get_one_transition_from(aalta_formula *f)
	{
		bool ret = solver_->solve_by_assumption(f);
		if (ret)
		{
			Transition *res = solver_->get_transition();
			return res;
		}
		return NULL;
	}

	void LTLfChecker::push_formula_to_explored(aalta_formula *f)
	{
		solver_->block_formula(f);
	}

	void LTLfChecker::push_uc_to_explored()
	{
		solver_->block_uc();
	}
}