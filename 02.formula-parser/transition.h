/**
 * File:   transition.h
 * Author: Yongkang Li
 *
 * Created on June 26, 2023, 17:05 AM
 */

#ifndef TRANSITION_H
#define TRANSITION_H

#include "formula/aalta_formula.h"

namespace aalta
{
	class Transition
	{
	public:
		typedef std::vector<aalta_formula *> af_vec;
		Transition(aalta_formula *label, aalta_formula *next)
			: label_(label), next_(next) {}
		Transition(af_vec &labels, af_vec &nexts)
			: label_(formula_from(labels)), next_(formula_from(nexts)) {}
		~Transition() {} // TODO: how to delete label and next?
		inline aalta_formula *label() { return label_; }
		inline aalta_formula *next() { return next_; }
		static Transition *make_transition(aalta_formula *label, aalta_formula *next)
		{
			Transition *t = new Transition(label, next);
			return t;
		}
		static Transition *make_transition(af_vec &labels, af_vec &nexts)
		{
			Transition *t = new Transition(labels, nexts);
			return t;
		}

	private:
		aalta_formula *label_;
		aalta_formula *next_;
	};
}

#endif