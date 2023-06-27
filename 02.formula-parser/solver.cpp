/**
 * File:   solver.cpp
 * Author: Yongkang Li
 *
 * Created on June 27, 2023, 09:28 AM
 */

#include "solver.h"

namespace aalta
{

    ///////////inline functions
    inline bool Solver::clauses_added(aalta_formula *f)
    {
        if (clauses_added_.find(f) != clauses_added_.end())
            return true;
        return false;
    }

    inline void Solver::mark_clauses_added(aalta_formula *f)
    {
        clauses_added_.insert(f);
    }

    inline void Solver::build_formula_map(aalta_formula *f)
    {
        // for !a, use -id (a) rather than id (!a);
        if (f->oper() == e_not)
            formula_map_.insert(std::pair<int, aalta_formula *>(-f->r_af()->id(), f));
        else
            formula_map_.insert(std::pair<int, aalta_formula *>(f->id(), f));
    }

    inline int Solver::SAT_id(aalta_formula *f)
    {
        // for !a, use -id (a) rather than id (!a);
        if (f->oper() == e_not)
            return -f->r_af()->id();
        return f->id();
    }

    inline bool Solver::is_label(aalta_formula *f)
    {
        return (f->oper() == e_not || f->oper() > e_undefined);
    }

    inline bool Solver::is_next(aalta_formula *f)
    {
        return (f->oper() == e_next || f->oper() == e_w_next);
    }

    inline void Solver::terminate_with_unsat()
    {
        unsat_forever_ = true;
    }
}