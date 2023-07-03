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
    // add LitId represented by id into `assumption_`
    void InvSolver::update_assumption_for_constraint(int id)
    {
        assumption_.push(id_to_lit(id));
    }

    // change last level element in `assumption_` from negative to positive, or vice verisa
    //                                            from `l` to `l^1`, represent `x` to `-x`  
    void InvSolver::disable_frame_and()
    {
        Minisat::Lit l = assumption_.last();
        // TODO: replace with equivalent exp: `l = ~l` or `l ^= 1`, because the return value of `last()` func is a reference!
        assumption_.pop();
        assumption_.push(~l);
    }
}