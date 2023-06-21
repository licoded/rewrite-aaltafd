/* 
 * File:   aalta_formula.cpp
 * Author: Yongkang Li
 *
 * Created on June 22, 2023, 10:40 AM
*/

#include "aalta_formula.h"
#include "ltlparser/trans.h"

namespace aalta {
    aalta_formula::aalta_formula() {} // new 时调用

    aalta_formula::aalta_formula(const aalta_formula &orig) // 拷贝构造函数
    {
        *this = orig; // 浅拷贝
    }

    aalta_formula::aalta_formula(OperatorType op,
                                 aalta_formula *left,
                                 aalta_formula *right)
        : op_(op),
          left_(left),
          right_(right) {}

    aalta_formula::aalta_formula(const char *input)
    {
        aalta_formula(getAST(input), false);
    }

    aalta_formula::aalta_formula(const ltl_formula *formula, bool is_not)
    {
        // build(formula, is_not);
    }

    aalta_formula::~aalta_formula() {}
} // namespace aalta_formula