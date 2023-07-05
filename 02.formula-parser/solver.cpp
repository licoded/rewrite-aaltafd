/**
 * File:   solver.cpp
 * Author: Yongkang Li
 *
 * Created on June 27, 2023, 09:28 AM
 */

#include "solver.h"
#include "debug.h"

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
     * @result A set of all formulas of ids in v
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
        if (!uc_on_)
            return;
        std::vector<int> uc = get_uc();
        af_prt_set ands = formula_set_of(uc);
        if (ands.empty())
            terminate_with_unsat(); // TODO: Why? What does empty uc means?
                                    // My assumption is, if uc=empty it indicates that the formulas to be check is_SAT is UNSAT forever.
        else
            block_elements(ands);
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

    int Solver::SAT_id_of_next(aalta_formula *f)
    {
        x_map::iterator it = X_map_.find(f->id());
        assert(it != X_map_.end());
        return it->second;
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
        af_list.clear(),
            af_s_list.clear(),
            sat_id_list.clear(),
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
                if ((*it)->is_wider_globally())
                    assumption_.push(id_to_lit(get_SAT_id(*it)));
            }
            else
                af_list.push_back(*it),
                    af_s_list.push_back((*it)->to_string()),
                    sat_id_list.push_back(get_SAT_id(*it)),
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

    // for each pair (Xa, X!a), (XXa, XX!a).., generate equivalence Xa<-> !X!a, XXa <-> !XX!a
    void Solver::add_X_conflicts()
    {
        std::vector<std::pair<int, int>> pairs = get_conflict_literal_pairs();
        for (int i = 0; i < pairs.size(); i++)
            add_X_conflict_for_pair(pairs[i]);
    }

    // collect all id pairs like (a, !a) from formula_map_
    std::vector<std::pair<int, int>>
    Solver::get_conflict_literal_pairs()
    {
        std::vector<std::pair<int, int>> res;
        for (formula_map::iterator it = formula_map_.begin(); it != formula_map_.end(); it++)
        {
            if (it->first < 0)
                continue;
            formula_map::iterator it2 = formula_map_.find(-(it->first));
            if (it2 != formula_map_.end())
                res.push_back(std::pair<int, int>(it->second->id(), it2->second->id()));
        }
        return res;
    }

    // given \@ pa = (a, !a), add equivalence for Xa <-> !X!a, and recursively XXa <-> !XX!a ...
    void Solver::add_X_conflict_for_pair(std::pair<int, int> &pa)
    {
        x_map::iterator it, it2;
        it = X_map_.find(pa.first);
        it2 = X_map_.find(pa.second);
        if (it != X_map_.end() && it2 != X_map_.end())
        {
            add_equivalence(it->second, -it2->second);
            std::pair<int, int> pa = std::pair<int, int>(it->second, it2->second);
            add_X_conflict_for_pair(pa);
        }
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

        //     null    || true or false || has added
        if (f == nullptr || clauses_added(f))
            return;
        assert(f->oper() != e_w_next);

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

    /**
     * EFFECT: convert `af *f` to propositional set, and then invoke SAT solver
     *
     * solve by taking the assumption of the CONJUNCTIVE formula f
     * If \@global is true, take the assumption with only global conjuncts of f
     *
     * Just ignore global now, think it is always false
     *
     * @result is_SAT - bool
     */
    bool Solver::solve_by_assumption(aalta_formula *f, bool global)
    {
        assert(!unsat_forever_);
        get_assumption_from(f, global);
        return solve_assumption(); // inherit from `AaltaSolver` class
    }

    // check whether the formula \@ f can be the last state (tail)
    // used in `sat_once()` func
    bool Solver::check_tail(aalta_formula *f)
    {
        get_assumption_from(f);
        af_list.push_back(aalta_formula::TAIL()),
            af_s_list.push_back(aalta_formula::TAIL()->to_string()),
            sat_id_list.push_back(tail_),
            assumption_.push(id_to_lit(tail_));
        // selected_assumption
        for(auto fid:sat_id_list)
        {
            dout << aalta_formula::get_af_by_SAT_id(fid)->to_string() << std::endl;
        }
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
            if ((*it) == 0) // in shrink_model/shrink_to_coi(), `=0` means clear/remove/delete this item
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
                                // NOTE: why f == NULL while (*it) != 0? Maybe the id is temporarily generated when/in add_clauses_for!
                                //       We know some afs are created temporarily. They are not recorded in all_afs.
                                //       e.g. X(a U b) for `a U b`!!!
                                // NOTE: why judge `(*it) > 0, why don't deal with negative ones?
                                //       Because we just care about positive formulas/assumps
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
     * @brief TODO: I can't understand now!
     */
    void Solver::shrink_to_coi(std::vector<int> &assign)
    {
        std::vector<int> coi = coi_of_assumption();
        for (int i = 0; i < assign.size(); i++)
        {
            if (i < coi.size())
            {
                if (coi[i] == 0) // is not a coi
                    assign[i] = 0;
            }
            else
                assign[i] = 0;
        }
    }

    // do nothing, empty now.
    void Solver::shrink_to_partial(std::vector<int> &assign)
    {
        //@TO BE DONE
    }
}