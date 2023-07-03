/**
 * File:   invsolver.cpp
 * Author: Yongkang Li
 *
 * Created on July 02, 2023, 13:56 PM
 */

#include "invsolver.h"
#include <vector>

namespace aalta
{
    // change last level element in `assumption_` from negative to positive, or vice verisa
    //                                            from `l` to `l^1`, represent `x` to `-x`
    void InvSolver::disable_frame_and()
    {
        Minisat::Lit l = assumption_.last();
        // TODO: replace with equivalent exp: `l = ~l` or `l ^= 1`, because the return value of `last()` func is a reference!
        assumption_.pop();
        assumption_.push(~l);
    }

    /**
     * CONCLUSION: add_clause -- \\/ /\ frame[i][j] in frame (param)
     * 
     * (0<=i<frame_len) cluase_flag[i] -> /\ frame[i][j]
     * add_clause -- \\/ clause_flag[i] 
     * 
     * TODO: I think we should replace `add_clause()` with `add_equivalence()`
     *          - may needn't to do this, so the old codes don't do this replacement?
     *              - but I don't know this exactly or deeply!
    */
    void InvSolver::add_clauses_for_frame_or(Frame &frame)
    {
        std::vector<int> v;
        for (int i = 0; i < frame.size(); i++)
        {
            int clause_flag = new_var();
            v.push_back(clause_flag);
            for (int j = 0; j < frame[i].size(); j++)  // clause_flag[i] -> /\ frame[i][j]
                add_clause(-clause_flag, frame[i][j]); // clause_flag[i] ->    frame[i][j]
        }
        add_clause(v); // \/ clause_flag[i]
    }

    /**
     * CONCLUSEION: add_clause -- ! \\/ /\ frame[i][j] in frame (param)
     * 
     * (0<=i<frame_len) frame_flag -> /\ ! /\ frame[i][j]
     *                           ==== ! \\/ /\ frame[i][j]
     * NOTE: with `assumption_.push(id_to_lit(frame_flag))` ==== `add_clause(frame_flag)`
    */
    void InvSolver::add_clauses_for_frame_and(Frame &frame)
    {
        int frame_flag = new_var();
        for (int i = 0; i < frame.size(); i++)
        {
            std::vector<int> v;
            for (int j = 0; j < frame[i].size(); j++)
                v.push_back(-frame[i][j]);
            v.push_back(-frame_flag);
            add_clause(v);  // \/ !(frame[i][j]) \/ !frame_flag
                            // frame_flag -> \/ !frame[i][j]
                            //               ! /\ frame[i][j]
        }
        /**
         * NOTE: this is for assumption_, which is different to add_clause!
         * but the final effect may be the same, so 
         * TODO: I think we should replace this with `add_clause()`
         * but why id_to_lit? Oh, I know. Because `add_clause()` also do id_to_lit before push.
        */
        assumption_.push(id_to_lit(frame_flag));
    }
}