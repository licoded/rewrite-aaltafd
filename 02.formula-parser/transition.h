/**
 * File:   transition.h
 * Author: Yongkang Li
 *
 * Created on June 26, 2023, 17:05 AM
 */

#ifndef TRANSITION_H
#define TRANSITION_H

#include "formula/aalta_formula.h"

namespace aalta {
	class Transition
	{
	public:
		Transition(aalta_formula *label, aalta_formula *next)
			: label_(label), next_(next) {}
		~Transition() {} // TODO: how to delete label and next?
		inline aalta_formula *label() { return label_; }
		inline aalta_formula *next() { return next_; }

	private:
		aalta_formula *label_;
		aalta_formula *next_;
	};
}

#endif