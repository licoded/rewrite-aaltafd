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

    /**
     * used in block_uc() func  --  `af_prt_set ands = formula_set_of(uc);`
     */
    aalta_formula::af_prt_set Solver::formula_set_of(std::vector<int> &v)
    {
        af_prt_set res;
        for (std::vector<int>::iterator it = v.begin(); it != v.end(); it++)
        {
            aalta_formula *f = formula_of(*it);
            if (f != NULL)
                res.insert(f);
        }
        return res;
    }

    void Solver::block_formula(aalta_formula *f)
    {
        af_prt_set ands = f->to_set();
        block_elements(ands);
    }

    void Solver::block_uc()
    {
        if (uc_on_)
        {
            std::vector<int> uc = get_uc();
            af_prt_set ands = formula_set_of(uc);
            if (ands.empty())
            {
                terminate_with_unsat(); // why? how to judge it!
                return;
            }
            block_elements(ands);
        }
    }

    /**
     * add X(f) when f is U(Until) or R(Release)
     */
    void Solver::build_X_map(aalta_formula *f)
    {
        assert(f->is_U_or_R());
        /**
         * TODO: I think the following lines can be replaced with more concise codes.
         *          - Since `insert()` will not take effect if exists.
         *          - But there is a `++max_used_id_` operation
         *              - I think it is OK, the `++max_used_id_` operation will just occupy empty/unused id number
         * TODO: The `X_map_` and `X_reverse_map_` insertion can be extracted into a func.
         */
        if (X_map_.find(f->id()) != X_map_.end())
            return;
        X_map_.insert({f->id(), ++max_used_id_});
        X_reverse_map_.insert({max_used_id_, f});
    }

    // set X_map_ in the input-formula level
    void Solver::build_X_map_priliminary(aalta_formula *f)
    {
        if (f == nullptr)
            return;
        if (f->is_next())
        {
            /**
             * TODO: I think this replacement is equivalent, but not very sure.
             *  - Can it repeat? And if repeats, do the two are the same? I think they are the same.
             */
            X_map_.insert({f->r_id(), f->id()});
        }
        build_X_map_priliminary(f->l_af());
        build_X_map_priliminary(f->r_af());
    }

    /**
     * add_clause( ![ /\ X(vi)]  ===  \/ !X(vi) )
     * check the existence of X(vi) before add_clause()
     */
    void Solver::block_elements(const af_prt_set &ands)
    {
        // if there is a conjuct A in f such that (A, X A) is not founded in X_map_, then discard blocking f
        if (block_discard_able(ands))
            return;
        std::vector<int> v;
        for (af_prt_set::const_iterator it = ands.begin(); it != ands.end(); it++)
            v.push_back(-SAT_id_of_next(*it));
        add_clause(v);
    }

    /**
     * check the existence of X(vi)
     */
    bool Solver::block_discard_able(const af_prt_set &ands)
    {
        for (af_prt_set::const_iterator it = ands.begin(); it != ands.end(); it++)
        {
            if (X_map_.find((*it)->id()) == X_map_.end())
                return true;
        }
        return false;
    }

    /**
     * @brief Set assumption_ of SAT solver from f. \
     * @brief If global is true, set assumption_ with only global parts of f \
     * @brief \
     * @brief Just ignore global now, think it is always false
     */
    void Solver::get_assumption_from(aalta_formula *f, bool global)
    {
        assumption_.clear();
        af_prt_set ands = f->to_set();
        /**
         * explain for `id_to_lit(get_SAT_id(*it)`
         *      - *it is `af*`
         *      - get_SAT_id: convert `af*` to `int id`
         *      - id_to_lit: conver `int id` to `lit`
         */
        for (af_prt_set::iterator it = ands.begin(); it != ands.end(); it++)
        {
            if (global)
            {
                if ((*it)->is_globally())
                    assumption_.push(id_to_lit(get_SAT_id(*it)));
            }
            else
                assumption_.push(id_to_lit(get_SAT_id(*it)));
        }
        // don't forget tail!!
        if (global)
            /**
             * TODO: why add TAIL when global is true?
             *          - I have look up the codes, this case -- `global == true` only used in heuristics part of `dfs_check()`
             *          - So just needn't to care about it now!
             */
            assumption_.push(id_to_lit(tail_));
    }

    /**
     * @brief iter `vec<Lit> assumption_` and exec `coi_of`
     *
     * @return `vector<int> &res`, ???
     */
    std::vector<int> Solver::coi_of_assumption()
    {
        std::vector<int> res(nVars(), 0);
        for (int i = 0; i < assumption_.size(); i++)
        {
            int id = lit_to_id(assumption_[i]);
            assert(id != 0);
            coi_of(id, res);
        }
        return res;
    }

    /**
     * @brief merge the coi/formula of id into `vector<int> &res`;
     *
     * @param id int
     * @param &res vector<int>
     */
    void Solver::coi_of(int id, std::vector<int> &res)
    {
        coi_map::iterator it = coi_map_.find(abs(id));
        if (it != coi_map_.end())
            coi_merge(res, it->second);
        else // check whether id represent a literal or Next
        {
            assert(res.size() >= abs(id));
            res[abs(id) - 1] = 0; // just reset the value
            aalta_formula *f = formula_of(id);
            if (f != NULL)
            {
                // COI includes only atoms
                if (f->oper() > e_undefined || f->oper() == e_not || f->oper() == e_next)
                    res[abs(id) - 1] = 1; // TODO: Why don't care about negative or positive?
            }
        }
    }

    /**
     * @brief `to |= from`, will extend the length of `to` if needed
     *
     * @param &to vector<int>
     * @param &from vector<int>
     */
    void Solver::coi_merge(std::vector<int> &to, std::vector<int> &from)
    {
        if (to.size() < from.size())
            to.resize(from.size(), 0); // resize() only influences the new added elements
        for (int i = 0; i < from.size(); i++)
            to[i] |= from[i];
    }

    /**
     * @param id int
     * @return the formula corresponding to \@ id
     */
    aalta_formula *Solver::formula_of(int id)
    {
        formula_map::iterator it = formula_map_.find(id);
        if (it != formula_map_.end())
            return it->second;
        return NULL;
    }

    /**
     * @param id int
     * @return the next inner of the formula corresponding to \@ id
     */
    aalta_formula *Solver::formula_of_next_inner(int id)
    {
        x_reverse_map::iterator it = X_reverse_map_.find(id);
        if (it != X_reverse_map_.end())
            return it->second;
        return NULL;
    }

    // generate clauses of SAT solver
    void Solver::generate_clauses(aalta_formula *f)
    {
        add_clauses_for(f);
        add_X_conflicts();
    }

    /**
     * add clauses for the formula f into SAT solver
     * It's a recursive func.
     *
     * NOTE: `U` and `R` still exists after all transfers!
     */
    void Solver::add_clauses_for(aalta_formula *f)
    {
        // We assume that
        // 1) the id of f is greater than that of its any subformula // TODO: check it, and why assume this?
        // 2) atoms and X subformulas are considered propositions
        // 3) be careful about !a, we should encode as -id (a) rather than id (!a)
        // 4) for a Global formula Gf, we directly add id(Gf) into the clauses, and will not consider it in assumptions // TODO: seems outdated, not impl
        // We also build the X_map and formula_map during the process of adding clauses
        assert(f->oper() != e_w_next);
        //     null    || true or false || has added
        if (f == nullptr || f->is_tf() || clauses_added(f))
            return;

        int id; // used for temporary subformula
        if (f->is_U_or_R())
            build_X_map(f);
        build_formula_map(f);
        switch (f->oper())
        {
        case e_until:                                                          // A U B = B \/ (A /\ !Tail /\ X (A U B))
            id = ++max_used_id_;                                               // id of `A /\ !Tail /\ X (A U B)` or `!Tail /\ X (F B)` -- if f->is_future()
            add_equivalence_wise(false, get_SAT_id(f), {get_r_SAT_id(f), id}); // A U B <-> B \/ id

            if (!f->is_future())
                add_equivalence_wise(true, id, {get_l_SAT_id(f), -tail_, SAT_id_of_next(f)}); // id <-> A /\ !Tail /\ X (A U B)
            else                                                                              // F B = B \/ (!Tail /\ X (F B))
                add_equivalence_wise(true, id, {-tail_, SAT_id_of_next(f)});                  // id <-> !Tail /\ X (F B)
            break;
        case e_release:                                                       // A R B = B /\ (A \/ Tail \/ X (A R B))
            id = ++max_used_id_;                                              // id of `A \/ Tail \/ X (A R B)` or `Tail \/ X (G B)` -- if f->is_globally()
            add_equivalence_wise(true, get_SAT_id(f), {get_r_SAT_id(f), id}); // A R B <-> B /\ id

            if (!f->is_globally())
                add_equivalence_wise(false, id, {get_l_SAT_id(f), tail_, SAT_id_of_next(f)}); // id <-> A \/ Tail \/ X (A R B)
            else                                                                              // G B = B /\ (Tail \/ X (G B))
                add_equivalence_wise(false, id, {tail_, SAT_id_of_next(f)});                  // id <-> Tail \/ X (G B)
            break;
        case e_and:
        case e_or:
            add_equivalence_wise(f->oper() == e_and, get_SAT_id(f), {get_l_SAT_id(f), get_r_SAT_id(f)}); // f <-> A /\ B
            break;
        case e_undefined:
        {
            cout << "Solver.cpp::add_clauses_for: Error reach here!\n";
            exit(0);
        }
        }
        add_clauses_for(f->l_af());
        add_clauses_for(f->r_af());
        mark_clauses_added(f);
    }

    // set up the COI map
    void Solver::coi_set_up(aalta_formula *f)
    {
        std::vector<int> ids;
        compute_full_coi(f, ids);
        // only Until, Release, And, Or formulas need to be recorded, why?
        // delete from coi_map_ all ids in \@ids
        shrink_coi(ids);
    }

    inline bool Solver::need_record(aalta_formula *f)
    {
        return f->oper() == e_until || f->oper() == e_release || f->oper() == e_or;
    }

    // used in `compute_full_coi()` func
    void Solver::coi_find_and_merge(aalta_formula *f, std::vector<int> &v)
    {
        coi_map::iterator it = coi_map_.find(f->id());
        assert(it != coi_map_.end());
        coi_merge(v, it->second);
    }

    /**
     * @param f: the formula
     * @param ids: the list of ids to be **deleted**
     */
    void Solver::compute_full_coi(aalta_formula *f, std::vector<int> &ids)
    {
        if (coi_map_.find(f->id()) != coi_map_.end())
            return;
        // only variables and Nexts need to be recorded
        // TODO: record to what variable? The following vector<int> v?
        std::vector<int> v(max_used_id_, 0);
        switch (f->oper())
        {
        case e_not: // id -> id for Literals
            if (f->r_af() != NULL)
            {
                compute_full_coi(f->r_af(), ids);
                coi_find_and_merge(f->r_af(), v);
                break;
            }

        /**
         * How about the inner of a U/R b?
         *   - We just do nothing to a and b? No, because there is no `break;`
         */
        case e_until:
        case e_release:
        {
            // add Xf in COI for Until/Release formula f
            x_map::iterator xit = X_map_.find(get_SAT_id(f));
            assert(xit != X_map_.end());
            v[xit->second - 1] = 1; // TODO: why `-1`?
            // NOTE: there is no `break;`
        }
        case e_and:
        case e_or:
            compute_full_coi(f->l_af(), ids);
            coi_find_and_merge(f->l_af(), v);

            compute_full_coi(f->r_af(), ids);
            coi_find_and_merge(f->r_af(), v);

            break;
        case e_undefined:
        {
            cout << "solver.cpp: Error reach here!\n";
            exit(0);
        }
        case e_next:
            if (f->r_af() != NULL)
                compute_full_coi(f->r_af(), ids);
        default:                // atoms
            v[f->id() - 1] = 1; // TODO: why `-1`?
            break;
        }

        /**
         * TODO: I want to merge the following two lines. But it seems I can't.
         *          - Because this is a recursive func!
         */
        coi_map_.insert({f->id(), v});
        if (!need_record(f))
            ids.push_back(f->id());
    }

    // delete from coi_map_ all ids in \@ids
    void Solver::shrink_coi(std::vector<int> &ids)
    {
        for (std::vector<int>::iterator it = ids.begin(); it != ids.end(); it++)
            coi_map_.erase(*it);
    }

    // solve by taking the assumption of the CONJUNCTIVE formula f
    // If \@global is true, take the assumption with only global conjuncts of f
    bool Solver::solve_by_assumption(aalta_formula *f, bool global)
    {
        assert(!unsat_forever_);
        get_assumption_from(f, global);
        return solve_assumption();
    }

    /**
     * @brief It doesn't include trigger the SAT solver, \
     * @brief you must invoke the SAT solver manually before execute this method.
     * @result A pair of <current, next>, which is extracted from the model of SAT solver
     */
    Transition *Solver::get_transition()
    {
        std::vector<int> assign = get_model();
        shrink_model(assign); // TODO: see the comments of `shrink_model` and understand it.

        std::vector<aalta_formula *> labels, nexts;
        for (std::vector<int>::iterator it = assign.begin(); it != assign.end(); it++)
        {
            if ((*it) == 0) // TODO: means what?
                continue;
            aalta_formula *f = formula_of(*it);
            if (f != NULL)
            {
                if (f->is_label())
                    labels.push_back(f);
                else if (f->is_next())
                    nexts.push_back(f->r_af());
            }
            else if ((*it) > 0) // handle the variables created for Next of Unitl, Release formulas
                                // TODO: why f == NULL while (*it) != 0? Maybe the id is temporarily generated when/in add_clauses_for!
                push_next_inner(*it, nexts);
        }

        return Transition::make_transition(labels, nexts);
    }

    /**
     * used in `get_transition()` func
    */
    void Solver::push_next_inner(int f_id, vector<aalta_formula *> &nexts)
    {
        aalta_formula *next_inner_af = formula_of_next_inner(f_id);
        if (next_inner_af != NULL)
            nexts.push_back(next_inner_af);
    }

    void Solver::shrink_model(std::vector<int> &assign)
    {
        /**
         * OLD COMMENTS:
         *      - Shrinking to COI is the MUST, otherwise it may happen that \phi is in the next state of \psi,
         *      - but \phi is not a subformula of \psi.
         * TODO: to understand the above old comments.
         */
        shrink_to_coi(assign);
        if (partial_on_) // NOTE: just needn't to care this, `shrink_to_partial` is empty
            shrink_to_partial(assign);
    }

    /**
     * @brief set assign[i] = 0, if/when coi[i] = 0 or coi[i] not exists
     */
    void Solver::shrink_to_coi(std::vector<int> &assign)
    {
        std::vector<int> coi = coi_of_assumption();
        for (int i = 0; i < assign.size(); i++)
        {
            if (i < coi.size())
                assign[i] &= coi[i]; // set assign[i] = 0, if/when coi[i] = 0
            else
                assign[i] = 0; // set assign[i] = 0, if/when coi[i] not exists
        }
    }

    // do nothing, empty now.
    void Solver::shrink_to_partial(std::vector<int> &assign)
    {
        //@TO BE DONE
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
     * Insert `af *f` into `formula_map_`, just {key: SAT_id of f, value: f}
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

    inline int Solver::get_l_SAT_id(aalta_formula *f)
    {
        assert(f->l_af() != nullptr);
        return get_SAT_id(f->l_af());
    }

    inline int Solver::get_r_SAT_id(aalta_formula *f)
    {
        assert(f->r_af() != nullptr);
        return get_SAT_id(f->r_af());
    }

    /**
     * Act as its name!
     */
    inline void Solver::terminate_with_unsat()
    {
        unsat_forever_ = true;
    }
}