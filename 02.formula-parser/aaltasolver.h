/**
 * File:   aaltasolver.h
 * Author: Yongkang Li
 *
 * Created on June 26, 2023, 16:30 AM
 */

#ifndef AALTA_SOLVER_H
#define AALTA_SOLVER_H

#include "minisat/core/Solver.h"
#include "formula/aalta_formula.h"
#include <vector>
#include <iostream>

namespace aalta
{
	class AaltaSolver : public Minisat::Solver
	{
	public:
		AaltaSolver() {
            init_solver();
        }
		AaltaSolver(bool verbose) : verbose_(verbose) {
            init_solver();
        }

		// variables
		bool verbose_;
		Minisat::vec<Minisat::Lit> assumption_; // Assumption for SAT solver
        std::vector<aalta_formula *> af_list;
        std::vector<std::string> af_s_list;
        std::vector<int> sat_id_list;

		// functions
        void init_solver();           // !false true
		bool solve_assumption();	  // invoke Solver::solveLimited() with assumption_
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
		inline void add_equivalence_wise(bool isAnd, int l, const std::vector<int> &); // l <-> /\ (vi) or l <-> \/ (vi)
	};

	///////////inline functions
	// l <-> r
    inline void AaltaSolver::add_equivalence(int l, int r)
    {
        add_clause(-l, r);
        add_clause(l, -r);
    }

    // l <-> r1 /\ r2
    inline void AaltaSolver::add_equivalence(int l, int r1, int r2)
    {
        add_clause(-l, r1);
        add_clause(-l, r2);
        add_clause(l, -r1, -r2);
    }

    // l<-> r1 /\ r2 /\ r3
    inline void AaltaSolver::add_equivalence(int l, int r1, int r2, int r3)
    {
        add_clause(-l, r1);
        add_clause(-l, r2);
        add_clause(-l, r3);
        add_clause(l, -r1, -r2, -r3);
    }

    /**
     * l <-> /\ (vi) or l <-> \/ (vi)
     * 
     * @param isAnd: true if l <-> /\ (vi), false if l <-> \/ (vi)
     * @param l: l
     * @param v: list of vi
    */
    inline void AaltaSolver::add_equivalence_wise(bool isAnd, int l, const std::vector<int> &v)
    {
        if(isAnd) // l <-> /\ (vi)
        {
            // ==== l -> /\ (vi)
            for (std::vector<int>::const_iterator it = v.begin(); it != v.end(); it++)
                add_clause(-l, *it); // l -> vi === !l \/ vi

            // ==== l <- /\ (vi) === [ \/ (!vi)] \/ l
            std::vector<int> new_v;
            new_v.push_back(l);
            for (std::vector<int>::const_iterator it = v.begin(); it != v.end(); it++)
                new_v.push_back(-*it);
            add_clause(new_v);
        }
        else    // l <-> \/ (vi)
        {
            // ==== l -> \/ (vi) === !l \/ [ \/ vi]
            std::vector<int> new_v;
            new_v.push_back(-l);
            for (std::vector<int>::const_iterator it = v.begin(); it != v.end(); it++)
                new_v.push_back(*it);
            add_clause(new_v);

            // ==== l <- \/ (vi) === !l -> ![ \/ (vi)]
            //                   === !l -> /\ (!vi)
            for (std::vector<int>::const_iterator it = v.begin(); it != v.end(); it++)
                add_clause(l, -*it); // !l -> !vi === l \/ !vi
        }
    }
}

#endif
