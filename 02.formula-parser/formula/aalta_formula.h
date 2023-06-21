/*
 * ltl_formula 变体:
 * 
 *     - 表达式中无 <->, ->
 *     - NNF: !只会出现在原子前
 * 
 * File:   aalta_formula.cpp
 * Author: Yongkang Li
 *
 * Created on June 22, 2023, 10:40 AM
*/

#ifndef AALTA_FORMULA_H
#define AALTA_FORMULA_H

#include "ltlparser/ltl_formula.h"

namespace aalta {
    /* 操作符类型 */
    enum OperatorType
    {
        True,
        False,
        Literal,
        Not,
        Or,
        And,
        Next,
        WNext, // weak Next, for LTLf
        Until,
        Release,
        Undefined
    };

    class aalta_formula; // 前置声明
    class aalta_formula
    {
    private:
        ////////////
        //成员变量//
        //////////////////////////////////////////////////
        int op_ = Undefined; // 操作符or操作数(原子atom)
        aalta_formula *left_ = nullptr; // 操作符左端公式
        aalta_formula *right_ = nullptr; // 操作符右端公式
        // int length_; //公式长度
        // aalta_formula *unique_ = nullptr; // 指向唯一指针标识
        // aalta_formula *simp_ = nullptr; // 指向化简后的公式指针
        static std::vector<std::string> names; // 存储操作符的名称以及原子变量的名称
        static std::unordered_map<std::string, int> ids; // 名称和对应的位置映射
        //////////////////////////////////////////////////

    public:
        aalta_formula(); // new 时调用
        aalta_formula(const aalta_formula& orig); // 拷贝构造函数
        aalta_formula(OperatorType op, aalta_formula *left, aalta_formula *right);
        aalta_formula(int atom_id);
        aalta_formula(const char *input);
        aalta_formula(const ltl_formula *formula, bool is_not = false);
        ~aalta_formula();
        void build (const ltl_formula *formula, bool is_not = false);
        void build_atom(const char *name, bool is_not = false);
    };
} // namespace aalta_formula

#endif	/* AALTA_FORMULA_H */