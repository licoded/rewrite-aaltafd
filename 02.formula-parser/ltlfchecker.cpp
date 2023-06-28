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
		visited_.push_back(f);

		if (detect_unsat())
			return false;
		if (sat_once(f))
			return true;
		if (f->is_globally())
		{
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

		// The SAT solver cannot return f as well
		push_formula_to_explored(f);

		while (true)
		{
			if (detect_unsat())
				return false;

			Transition *t = get_one_transition_from(f);
			if (t != NULL)
			{
				if (dfs_check(t->next()))
				{
					delete t;
					return true;
				}
			}
			else // cannot get new states, that means f is not used anymore
			{
				visited_.pop_back();
				push_uc_to_explored();
				delete t;
				return false;
			}
		}
		visited_.pop_back();
		return false;
		/**
		 * TODO:
		 * Why first `pop_back()` then `return false;`?
		 * Why not just do `return false;` without `pop_back()`
		*/
	}
}