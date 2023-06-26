/**
 * File:   aaltasolver.h
 * Author: Yongkang Li
 *
 * Created on June 26, 2023, 17:11 AM
 */

#include "aaltasolver.h"
#include <iostream>
#include <vector>

#define l_True  (Minisat::lbool((uint8_t)0)) // gcc does not do constant propagation if these are real constants.
#define l_False (Minisat::lbool((uint8_t)1))
#define l_Undef (Minisat::lbool((uint8_t)2))

namespace aalta
{
    Minisat::Lit AaltaSolver::SAT_lit(int id)
    {
        assert(id != 0);
        int var = abs(id) - 1;
        while (var >= nVars())
            newVar();
        return ((id > 0) ? Minisat::mkLit(var) : ~Minisat::mkLit(var));
    }

    int AaltaSolver::lit_id(Minisat::Lit l)
    {
        if (sign(l))
            return -(var(l) + 1);
        else
            return var(l) + 1;
    }

    bool AaltaSolver::solve_assumption()
    {
        Minisat::lbool ret = solveLimited(assumption_);
        if (ret == l_True)
            return true;
        else if (ret == l_Undef)
            exit(0);
        return false;
    }

    // return the model from SAT solver when it provides SAT
    std::vector<int> AaltaSolver::get_model()
    {
        std::vector<int> res;
        res.resize(nVars(), 0);
        for (int i = 0; i < nVars(); i++)
        {
            if (model[i] == l_True)
                res[i] = i + 1;
            else if (model[i] == l_False)
                res[i] = -(i + 1);
        }
        return res;
    }

    // return the UC from SAT solver when it provides UNSAT
    std::vector<int> AaltaSolver::get_uc()
    {
        std::vector<int> reason;
        for (int k = 0; k < conflict.size(); k++)
        {
            Minisat::Lit l = conflict[k];
            reason.push_back(-lit_id(l));
        }
        return reason;
    }

    void AaltaSolver::add_clause(std::vector<int> &v)
    {
        Minisat::vec<Minisat::Lit> lits;
        for (std::vector<int>::iterator it = v.begin(); it != v.end(); it++)
            lits.push(SAT_lit(*it));
        addClause(lits);
    }

    void AaltaSolver::add_clause(int id)
    {
        std::vector<int> v{id};
        add_clause(v);
    }

    void AaltaSolver::add_clause(int id1, int id2)
    {
        std::vector<int> v{id1, id2};
        add_clause(v);
    }

    void AaltaSolver::add_clause(int id1, int id2, int id3)
    {
        std::vector<int> v{id1, id2, id3};
        add_clause(v);
    }

    void AaltaSolver::add_clause(int id1, int id2, int id3, int id4)
    {
        std::vector<int> v{id1, id2, id3, id4};
        add_clause(v);
    }

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
}
