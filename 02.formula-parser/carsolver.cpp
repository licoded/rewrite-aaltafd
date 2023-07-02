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

    std::vector<int> CARSolver::get_selected_uc()
    {
        std::vector<int> uc = get_uc();
        std::vector<int> res;
        for (int i = 0; i < uc.size(); i++)
        {
            if (selected_assumption_.find(uc[i]) != selected_assumption_.end()) // TODO: why check this, I think it's very easy to get empty ERROR!
                res.push_back(uc[i]);
        }
        assert(!res.empty());
        return res;
    }

    void CARSolver::create_flag_for_frame (int frame_level)
	{
		assert (frame_flags_.size () == frame_level);
		frame_flags_.push_back (++max_used_id_);
	}

    bool CARSolver::check_final(aalta_formula *f)
    {
        set_selected_assumption(f); // just used in get_selected_uc() func
        return Solver::check_tail(f);
    }

    void CARSolver::set_selected_assumption(aalta_formula *f)
    {
        selected_assumption_.clear();
        af_prt_set ands = f->to_set();
        for (af_prt_set::iterator it = ands.begin(); it != ands.end(); it++)
            selected_assumption_.insert(get_SAT_id(*it));
    }
}