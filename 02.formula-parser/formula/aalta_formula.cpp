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

    aalta_formula* aalta_formula::unique()
    {
        // TODO: modify this to archieve/get unique pointer
        //       > leverage hashset to ensure this
        return new aalta_formula(*this);
    }

    inline int
    aalta_formula::get_id_by_name(const char *name)
    {
        int id;
        const auto& it = name_id_map.find(name);
        if (it == name_id_map.end())
        { // 此变量名未出现过，添加之
            id = names.size();
            name_id_map[name] = id;
            names.push_back(name);
        }
        else
            id = it->second;
        return id;
    }

    /**
     * 添加原子变量
     * @param name
     * @param is_not
     */
    inline void
    aalta_formula::build_atom(const char *name, bool is_not)
    {
        int id = get_id_by_name(name); // will add name to names, if no exist/added before
        
        if (is_not)
            op_ = e_not, right_ = aalta_formula(id).unique();
        else
            op_ = id;
    }

    /**
     * 将ltl_formula转成aalta_formula结构，
     * 并处理！运算，使其只会出现在原子前
     * @param formula
     * @param is_not 标记此公式前是否有！
     */
    void
    aalta_formula::build(const ltl_formula *formula, bool is_not)
    {
        aalta_formula *tmp_left, *tmp_right;
        if (formula == NULL)
            return;
        switch (formula->_type)
        {
            case eTRUE: // True - [! True = False]
                op_ = is_not ? e_false : e_true;
                break;
            case eFALSE: // False - [! False = True]
                op_ = is_not ? e_true : e_false;
                break;
            case eLITERAL: // atom
                build_atom (formula->_var, is_not);
                break;
            case eNOT:
                build(formula->_right, is_not^1);
                break;
            case eNEXT: // Xa -- [!(Xa) = N(!a)]
                op_ = is_not ? e_w_next : e_next;
                right_ = new aalta_formula(formula->_right, is_not);
                break;
            case eWNEXT: // Na -- [!(Na) = X(!a)]
                op_ = is_not ? e_next : e_w_next;
                right_ = new aalta_formula(formula->_right, is_not);
                break;
            case eGLOBALLY: // G a = False R a -- [!(G a) = True U !a]
                if (is_not)
                    op_ = e_until, left_ = TRUE();
                else
                    op_ = e_release, left_ = FALSE();
                right_ = aalta_formula(formula->_right, is_not).unique();
                break;
            case eFUTURE: // F a = True U a -- [!(F a) = False R !a]
                if (is_not)
                    op_ = e_release, left_ = FALSE();
                else
                    op_ = e_until, left_ = TRUE();
                right_ = aalta_formula(formula->_right, is_not).unique();
                break;
            case eUNTIL: // a U b -- [!(a U b) = !a R !b]
                op_ = is_not ? e_release : e_until;
                left_ = aalta_formula(formula->_left, is_not).unique();
                right_ = aalta_formula(formula->_right, is_not).unique();
                break;
            case eWUNTIL: // a W b = (G a) | (a U b) -- [!(a W b) = F !a /\ (!a R !b)]
                tmp_left = aalta_formula(formula->_left, is_not).unique();
                tmp_right = aalta_formula(formula->_right, is_not).unique();
                if (is_not)
                {
                    op_ = e_and;
                    left_ = &aalta_formula(e_until, TRUE(), tmp_left);
                    right_ = aalta_formula(e_release, tmp_left, tmp_right).unique();
                }
                else
                {
                    op_ = e_or;
                    left_ = &aalta_formula(e_release, FALSE(), tmp_left);
                    right_ = aalta_formula(e_until, tmp_left, tmp_right).unique();
                }
                break;
            case eRELEASE: // a R b -- [!(a R b) = !a U !b]
                op_ = is_not ? e_until : e_release;
                left_ = aalta_formula(formula->_left, is_not).unique();
                right_ = aalta_formula(formula->_right, is_not).unique();
                break;
            case eAND: // a & b -- [!(a & b) = !a | !b ]
                op_ = is_not ? e_or : e_and;
                left_ = aalta_formula(formula->_left, is_not).unique();
                right_ = aalta_formula(formula->_right, is_not).unique();
                break;
            case eOR: // a | b -- [!(a | b) = !a & !b]
                op_ = is_not ? e_and : e_or;
                left_ = aalta_formula(formula->_left, is_not).unique();
                right_ = aalta_formula(formula->_right, is_not).unique();
                break;
            case eIMPLIES: // a->b = !a | b -- [!(a->b) = a & !b]
                op_ = is_not ? e_and : e_or;
                left_ = &aalta_formula(formula->_left, is_not ^ 1);
                right_ = aalta_formula(formula->_right, is_not).unique();
                break;
            case eEQUIV:
            { // a<->b = (!a | b)&(!b | a) -- [!(a<->b) = (a & !b)|(!a & b)]
                ltl_formula *not_a = create_operation(eNOT, NULL, formula->_left);
                ltl_formula *not_b = create_operation(eNOT, NULL, formula->_right);
                ltl_formula *new_left = create_operation(eOR, not_a, formula->_right);
                ltl_formula *new_right = create_operation(eOR, not_b, formula->_left);
                ltl_formula *now = create_operation(eAND, new_left, new_right);
                *this = *(aalta_formula(now, is_not));
                destroy_node(not_a);
                destroy_node(not_b);
                destroy_node(new_left);
                destroy_node(new_right);
                destroy_node(now);
                break;
            }
            default:
                print_error("the formula cannot be recognized by aalta!");
                exit(1);
                break;
        }
    }

    /* 初始化静态变量 */
    // can't declare and define non-const static variables in the same time
    aalta_formula* aalta_formula::TRUE_ = nullptr;
    aalta_formula* aalta_formula::FALSE_ = nullptr;

    aalta_formula* aalta_formula::TRUE()
    {
        if (TRUE_ == nullptr)
            TRUE_ = aalta_formula(e_true).unique();
        return TRUE_;
    }
    aalta_formula* aalta_formula::FALSE()
    {
        if (FALSE_ == nullptr)
            FALSE_ = aalta_formula(e_false).unique();
        return FALSE_;
    }
} // namespace aalta_formula