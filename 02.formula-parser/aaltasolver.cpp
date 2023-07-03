/**
 * File:   aaltasolver.h
 * Author: Yongkang Li
 *
 * Created on June 26, 2023, 17:11 AM
 */

#include "aaltasolver.h"
#include <iostream>
#include <vector>

using namespace Minisat;

namespace aalta
{
    bool AaltaSolver::solve_assumption()
    {
        Minisat::lbool ret = solveLimited(assumption_);
        if (ret == l_Undef)
            exit(0);
        return (ret == l_True);
    }

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

    /**
     * return the UC from SAT solver when it provides UNSAT
     * NOTE: it is change into negative/opposite before added to reason/uc!
     *       TODO: but we don't need negative, we negative it in `add_clause_for_frame()` func.
     *             so why there is a negative/opposite operator?
     *             JUST SKIP it, needn't to understand so deeply/detailly!
     * NOTE: but still have the X problem, we should have X(uc) when try_satisfy
     *       - the X problem is also copied in `add_clause_for_frame()` func.
    */
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

}
