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
    /**
     * Check if `af *f` is in `clauses_added_`
    */
    inline bool Solver::clauses_added(aalta_formula *f)
    {
        if (clauses_added_.find(f) != clauses_added_.end())
            return true;
        return false;
    }

    /**
     * Insert `af *f` into `clauses_added_`
    */
    inline void Solver::mark_clauses_added(aalta_formula *f)
    {
        clauses_added_.insert(f);
    }

    /**
     * Insert `af *f` into `formula_map_`
    */
    inline void Solver::build_formula_map(aalta_formula *f)
    {
        /**
         * (OLD)COMMENTS: for !a, use `-id (a)` rather than `id (!a)`;
         * TODO: What is the difference of the two?
         *          - I just think the logic of `-id(a)` is more natural.
         *          - And the former may save a new id than the latter.
        */
        if (f->oper() == e_not)
            formula_map_.insert(std::pair<int, aalta_formula *>(-f->r_af()->id(), f));
        else
            formula_map_.insert(std::pair<int, aalta_formula *>(f->id(), f));
    }

    /**
     * Return the SAT_id of `af *f`
     * NOTE: for !a, use -id (a) rather than id (!a);
    */
    inline int Solver::get_SAT_id(aalta_formula *f)
    {
        // for !a, use -id (a) rather than id (!a);
        if (f->oper() == e_not)
            return -f->r_af()->id();
        return f->id();
    }

    inline void Solver::terminate_with_unsat()
    {
        unsat_forever_ = true;
    }
}