/**
 * File:   solver.cpp
 * Author: Yongkang Li
 *
 * Created on June 27, 2023, 09:28 AM
 */

#include "solver.h"

namespace aalta
{
    Solver::Solver(aalta_formula *f, bool verbose, bool partial_on, bool uc_on) : AaltaSolver(verbose), uc_on_(uc_on), partial_on_(partial_on), unsat_forever_(false)
    {
        max_used_id_ = f->id();
        tail_ = aalta_formula::TAIL()->id();
        build_X_map_priliminary(f);
        generate_clauses(f);
        coi_set_up(f);
    }

    // set X_map_ in the input-formula level
    void Solver::build_X_map_priliminary(aalta_formula *f)
    {
        if (f == nullptr)
            return;
        if (f->is_next())
        {
            if (X_map_.find(f->r_id()) == X_map_.end())
                X_map_.insert({f->r_id(), f->id()});
        }
        build_X_map_priliminary(f->l_af());
        build_X_map_priliminary(f->r_af());
    }

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
        formula_map_.insert({get_SAT_id(f), f}); // {key, value}
    }

    /**
     * Return the SAT_id of `af *f`
     * NOTE: for !a, use -id (a) rather than id (!a);
     *
     * TODO: Why use `inline` instead of `static inline` to define this function?
     *          - may because `static inline` cannot act as `inline` expectation
     */
    inline int Solver::get_SAT_id(aalta_formula *f)
    {
        /**
         * (OLD)COMMENTS: for !a, use `-id (a)` rather than `id (!a)`;
         * TODO: What is the difference of the two?
         *          - I just think the logic of `-id(a)` is more natural.
         *          - And the former may save a new id than the latter.
         */
        if (f->oper() == e_not)
            return -f->r_af()->id();
        return f->id();
    }

    /**
     * Act as its name!
     */
    inline void Solver::terminate_with_unsat()
    {
        unsat_forever_ = true;
    }
}