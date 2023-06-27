/**
 * File:   solver.h
 * Author: Yongkang Li
 *
 * Created on June 26, 2023, 16:29 AM
 */

#ifndef SOLVER_H
#define SOLVER_H

#include "aaltasolver.h"
#include "formula/aalta_formula.h"
#include "transition.h"
#include <iostream>
#include <vector>
#include <unordered_map>

using namespace std;

namespace aalta
{
	class Solver : public AaltaSolver
	{
	public:
		Solver(){};
		Solver(aalta_formula *f, bool verbose = false, bool partial_on = false, bool uc_on = true);
		~Solver(){};
		// solve by taking the assumption of the CONJUNCTIVE formula f.
		// If \@global is true, take the assumption with only global conjuncts of f
		bool solve_by_assumption(aalta_formula *f, bool global = false);
		// solve by taking the assumption in \@assumption_
		// bool solve_with_assumption ();

		// check whether the formula \@ f can be the last state (tail)
		bool check_tail(aalta_formula *f);
		// return a pair of <current, next>, which is extracted from the model of SAT solver
		Transition *get_transition();
		// add clause to block the CONJUNCTIVE formula f
		void block_formula(aalta_formula *f);

		// add clause to block set of explored states from UC.
		void block_uc();

		inline bool unsat_forever()
		{
			return unsat_forever_;
		}

		// solve by taking the assumption of global CONJUNCTIVE formula f
		inline bool solve_with_global_assumption(aalta_formula *f)
		{
			return solve_by_assumption(f, true);
		}

	protected:
		////////////members
		int tail_;		  // (OLD)COMMENTS: the integer used to represent Tail. It is fixed to be f->id ()+1. TODO: I don't think that it is fixed to be `f->id() + 1` now.
		int max_used_id_; // the maximum id used in the SAT solver

		typedef aalta_formula::af_prt_set af_prt_set;
		af_prt_set clauses_added_; // set of formulas whose clauses are already created.

		typedef unordered_map<int, int> x_map;
		x_map X_map_; // if (1, 2) is in X_map_, that means 2 = X 1;
		x_map N_map_; // if (1, 2) is in N_map_, that means 2 = N 1;
		typedef unordered_map<int, aalta_formula *> formula_map;
		// if (1, a) is in formula_map_, that means SAT_id (a) == 1
		// we need to store literals (including atoms), Next (WNext), Until, Release and Or
		formula_map formula_map_;
		typedef unordered_map<int, aalta_formula *> x_reverse_map;
		x_reverse_map X_reverse_map_; // if (4, f) is in the map, that means SAT_id (Xf) = 4, here f is a Until/Release formula

		typedef unordered_map<int, std::vector<int>> coi_map;
		coi_map coi_map_; // if (1, v) is in coi_map_, that means coi (1) = v;
		// TODO: elements of v=coi(i) are \/ or /\ ?

		/////flags
		// bool verbose_;  //default is false
		bool uc_on_;		 // use uc when it is true
		bool partial_on_;	 // use partial model when it is true
		bool unsat_forever_; // never call SAT solver when it is true, and report error

		//////////functions
		void build_X_map(aalta_formula *f);
		void build_X_map_priliminary(aalta_formula *f);
		int SAT_id_of_next(aalta_formula *f);	   // return the id of Xf used in SAT solver
		int SAT_id_of_weak_next(aalta_formula *f); // return the id of Nf used in SAT solver

		void block_elements(const af_prt_set &ands);
		bool block_discard_able(const af_prt_set &ands);
		aalta_formula::af_prt_set formula_set_of(std::vector<int> &v);

		// set assumption_ of SAT solver from \@ f. If \@ global is true, set assumption_ with only global parts of \@ f
		void get_assumption_from(aalta_formula *f, bool global = false);

		std::vector<int> coi_of_assumption();						  // get COI for assumptions
		void coi_of(int id, std::vector<int> &res);					  // get COI for the given \@id, results are stored in \@ res
		void coi_merge(std::vector<int> &to, std::vector<int> &from); // merge the coi information from \@ from to \@ to
		void generate_clauses(aalta_formula *);						  // generate claueses for SAT solver
		void add_clauses_for(aalta_formula *);						  // add clauses for the formula f into SAT solver
		// for each pair (Xa, X!a), (XXa, XX!a).., generate equivalence Xa<-> !X!a, XXa <-> !XX!a
		void add_X_conflicts();
		// collect all id pairs like (a, !a) from formula_map_
		std::vector<std::pair<int, int>> get_conflict_literal_pairs();
		// given \@ pa = (a, !a), add equivalence for Xa <-> !X!a, and recursively XXa <-> !XX!a ...
		void add_X_conflict_for_pair(std::pair<int, int> &pa);

		void shrink_to_coi(std::vector<int> &); // shrink the assignment to COI, i.e. relevant variables only
		void coi_set_up(aalta_formula *);
		inline bool need_record(aalta_formula *);
		void coi_find_and_merge(aalta_formula *f, std::vector<int> &v);
		void compute_full_coi(aalta_formula *f, std::vector<int> &ids);
		void shrink_coi(std::vector<int> &ids);
		void shrink_to_partial(std::vector<int> &); // shrink the assignment to paritial one
		aalta_formula *formula_of(int id);			// return the formula corresponding to \@ id

		void shrink_model(std::vector<int> &);

		///////////inline functions
		inline bool clauses_added(aalta_formula *f);
		inline void mark_clauses_added(aalta_formula *f);
		inline void build_formula_map(aalta_formula *f);
		inline int get_SAT_id(aalta_formula *f);
		inline int get_l_SAT_id(aalta_formula *f);
		inline int get_r_SAT_id(aalta_formula *f);
		inline void terminate_with_unsat();
	};
}

#endif
