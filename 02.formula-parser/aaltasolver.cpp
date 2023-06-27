/**
 * File:   aaltasolver.h
 * Author: Yongkang Li
 *
 * Created on June 26, 2023, 17:11 AM
 */

#include "aaltasolver.h"
#include <iostream>
#include <vector>

#define l_True (Minisat::lbool((uint8_t)0)) // gcc does not do constant propagation if these are real constants.
#define l_False (Minisat::lbool((uint8_t)1))
#define l_Undef (Minisat::lbool((uint8_t)2))

namespace aalta
{
    // return the model from SAT solver when it provides SAT
    std::vector<int> AaltaSolver::get_model()
    {
        std::vector<int> res(nVars(), 0);
        for (int i = 0; i < nVars(); i++)
        {
            if (model[i] == l_True)
                res[i] = i + 1;
            else if (model[i] == l_False)
                res[i] = -(i + 1);
            // TODO: Can it be l_Undef? I don't think so. But the codes indicate so!
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
            reason.push_back(-lit_to_id(l));
        }
        return reason;
    }

    Minisat::Lit AaltaSolver::id_to_lit(int id)
    {
        assert(id != 0);
        int var = abs(id) - 1;
        while (var >= nVars())
            newVar();
        // note: Minisat::Lit has overloaded `~` operator, it is equivalent to `^1`
        return ((id > 0) ? Minisat::mkLit(var) : ~Minisat::mkLit(var));
    }

    int AaltaSolver::lit_to_id(Minisat::Lit l)
    {
        if (sign(l))
            return -(var(l) + 1);
        else
            return var(l) + 1;
        /**
         * Why both `+1`? What is the 0th/1th item?
         *  - Oh, I also see `abs(id)-1` in `id_to_lit()`
         */
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
            reason.push_back(-lit_to_id(l));
        }
        return reason;
    }

    /**
     * All others overloading add_clause() all invoke this one!
     * All add_equivalence() finally invoke this one, too.
     *
     * 作用: 在 SAT solver 的“待满足条件”中加上条件  " \/ (vi) "
     */
    void AaltaSolver::add_clause(std::vector<int> &v)
    {
        Minisat::vec<Minisat::Lit> lits;
        for (std::vector<int>::iterator it = v.begin(); it != v.end(); it++)
            lits.push(id_to_lit(*it));
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
}
