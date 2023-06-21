/* 
 * File:   aalta_formula.cpp
 * Author: Yongkang Li
 *
 * Created on June 22, 2023, 10:40 AM
*/

#include "aalta_formula.h"
#include "ltlparser/trans.h"
#include <unordered_map>
#include <vector>

namespace aalta {
    aalta_formula::aalta_formula() {} // new 时调用

    aalta_formula::aalta_formula(const aalta_formula &orig) // 拷贝构造函数
    {
        *this = orig; // 浅拷贝
    }

    aalta_formula::aalta_formula(int atom_id)
        : op_(atom_id)
        {}
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
    // TODO: variable `is_not`, I want to change a more clear/articulate name
    {
        // build(formula, is_not);
    }

    aalta_formula::~aalta_formula() {}

    /**
     * 添加原子变量
     * @param name
     * @param is_not
     */
    inline void
    aalta_formula::build_atom(const char *name, bool is_not)
    {
        int id;
        const auto& it = ids.find(name);
        if (it == ids.end())
        { // 此变量名未出现过，添加之
            id = names.size();
            ids[name] = id;
            names.push_back(name);
        }
        else
        {
            id = it->second;
        }
        if (is_not)
            op_ = Not, right_ = aalta_formula(id);
        else
            op_ = id;
    }
} // namespace aalta_formula