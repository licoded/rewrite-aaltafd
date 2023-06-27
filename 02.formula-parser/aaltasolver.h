/**
 * File:   aaltasolver.h
 * Author: Yongkang Li
 *
 * Created on June 26, 2023, 16:30 AM
 */

#ifndef AALTA_SOLVER_H
#define AALTA_SOLVER_H

#include "minisat/core/Solver.h"
#include <vector>

namespace aalta
{
	class AaltaSolver : public Minisat::Solver
	{
	public:
		AaltaSolver() {}
		AaltaSolver(bool verbose) : verbose_(verbose) {}
		bool verbose_;

		Minisat::vec<Minisat::Lit> assumption_; // Assumption for SAT solver

		// functions
		bool solve_assumption();
		std::vector<int> get_model(); // get the model from SAT solver
		std::vector<int> get_uc();	  // get UC from SAT solver

		Minisat::Lit id_to_lit(int id);	// create the Lit used in SAT solver for the id.
		int lit_to_id(Minisat::Lit);	// return the id of SAT lit

		// 作用: 在 SAT solver 的“待满足条件”中加上条件
		void add_clause(int);
		void add_clause(int, int);
		void add_clause(int, int, int);
		void add_clause(int, int, int, int); 
		void add_clause(std::vector<int> &); // 添加的条件是: " \/ (vi) "

		// 作用: 在 SAT solver 的“待满足条件”中加上条件, 与上面 add_clause 的不同在于, 这里添加的都是'等价关系'形式的条件
		inline void add_equivalence(int l, int r); 					// l <-> r
		inline void add_equivalence(int l, int r1, int r2); 		// l <-> r1 /\ r2
		inline void add_equivalence(int l, int r1, int r2, int r3); // l <-> r1 /\ r2 /\ r3
	};
}

#endif
