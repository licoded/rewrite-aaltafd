/**
 * File:   carsolver.h
 * Author: Yongkang Li
 *
 * Created on July 02, 2023, 13:13 PM
 */

#include "carsolver.h"
#include <iostream>
#include <assert.h>
using namespace std;

namespace aalta
{
    /**
     * return SAT of `ψ ∧ xnf(φ)`, in which ψ = C[frame level] = ! /\ X(uc[i])
     * 
     * TODO: seems selected_assumption is not used here, can we remove it?
     *       But maybe it will be used after this func...
    */
    bool CARSolver::solve_with_assumption(aalta_formula *f, int frame_level)
    {
        assert(frame_level < frame_flags_.size());
        assert(!unsat_forever_);
        set_selected_assumption(f);
        
        // selected_assumption
        for(auto fid:selected_assumption_)
        {
            std::cout << aalta_formula::get_af_by_SAT_id(fid)->to_string() << std::endl;
        }

        get_assumption_from(f, false);  // f = φ
        assumption_.push(id_to_lit(frame_flags_[frame_level])); // ψ = C[frame level] = ! /\ X(uc[i])
        return solve_assumption(); // ψ ∧ xnf(φ)
    }

    /**
     * `add_clause()` -- frame_id -> ! /\ X(uc[i])
    */
    void CARSolver::add_clause_for_frame(std::vector<int> &uc, int frame_level)
    {
        assert(frame_level < frame_flags_.size());
        af_prt_set ands = formula_set_of(uc); // just for remove repeat and NULL items
        // if there is a conjuct A in f such that (A, X A) is not founded in X_map_, then discard blocking f
        if (block_discard_able(ands))
            return;
        /**
         * TODO: The following/remain codes can be replaced by `add_equivalence_wisely()` func!
        */
        std::vector<int> v;
        for (af_prt_set::const_iterator it = ands.begin(); it != ands.end(); it++)
            v.push_back(-SAT_id_of_next(*it));  // NOTE: next and negative !!!
        v.push_back(-(frame_flags_[frame_level]));
        add_clause(v);  // frame_id -> \/ !X(uc[i])
                        //        ==== ! /\ X(uc[i])
    }

    /**
     * return uc=get_uc(), but filterd with `selected_assumption_`
     * OR return get_uc() /\ selected_assumption_
    */
    std::vector<int> CARSolver::get_selected_uc()
    {
        std::vector<int> uc = get_uc();
        std::vector<int> res;
        for (int i = 0; i < uc.size(); i++)
        {
            /**
             * 需要筛选说明, `minisat::conflict` may contains both clauses and assumps, 而我们只需要在 assumps 中的
             * TODO: 另一个疑问, res很容易为空? 
             *          - 可能并不会, 因为 clauses 只添加了辅助 minisat 求解的等价关系, 
             *          - 相当于让他聪明一点, 多了解点知识, 但这些应该都是已知的真理, 不会错的
            */
            if (selected_assumption_.find(uc[i]) != selected_assumption_.end())
                res.push_back(uc[i]);
        }
        assert(!res.empty());
        return res;
    }

    void CARSolver::create_flag_for_frame(int frame_level)
    {
        assert(frame_flags_.size() == frame_level);
        frame_flags_.push_back(++max_used_id_);
    }

    bool CARSolver::check_final(aalta_formula *f)
    {
        set_selected_assumption(f); // just used in get_selected_uc() func
        return Solver::check_tail(f);
    }

    // selected_assumption_ = f->to_set(), which is splited by e_and operator
    void CARSolver::set_selected_assumption(aalta_formula *f)
    {
        selected_assumption_.clear();
        af_prt_set ands = f->to_set();
        for (af_prt_set::iterator it = ands.begin(); it != ands.end(); it++)
            selected_assumption_.insert(get_SAT_id(*it));
    }
}