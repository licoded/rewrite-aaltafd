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
#include <cstdlib>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace aalta {
    /* 操作符类型 */
    enum OperatorType
    {
        e_true,
        e_false,
        e_literal,
        e_not,
        e_or,
        e_and,
        e_next,
        e_w_next, // weak Next, for LTLf
        e_until,
        e_release,
        e_undefined
    };

    class aalta_formula; // 前置声明
    class aalta_formula
    {
    public:
        /* af公式的hash函数 */
        struct af_hash
        {

            size_t operator()(const aalta_formula &af) const
            {
                return af.hash_;
            }
        };

        /* af指针的hash函数 */
        struct af_prt_hash
        {

            size_t operator()(const aalta_formula *af_prt) const
            {
                // return size_t (af_prt);
                return af_prt->id_;
            }
        };

        /* af指针的hash函数 */
        struct af_prt_hash2
        {

            size_t operator()(const aalta_formula *af_prt) const
            {
                return af_prt->hash_;
            }
        };
        /* af指针的相等函数 */
        struct af_prt_eq
        {

            bool operator()(const aalta_formula *af_prt1, const aalta_formula *af_prt2) const
            {
                return *af_prt1 == *af_prt2;
            }
        };
        typedef std::unordered_set<aalta_formula *, af_prt_hash2, af_prt_eq> afp_set;

    private:
        ////////////
        //成员变量//
        //////////////////////////////////////////////////
        int op_; // 操作符or操作数(原子atom)
        aalta_formula *left_ = nullptr; // 操作符左端公式
        aalta_formula *right_ = nullptr; // 操作符右端公式
        // int length_; //公式长度
        aalta_formula *unique_ = nullptr; // 指向唯一指针标识
        // aalta_formula *simp_ = nullptr; // 指向化简后的公式指针
        static std::vector<std::string> names; // 存储操作符的名称以及原子变量的名称
        static std::unordered_map<std::string, int> name_id_map; // 名称和对应的位置映射
        static afp_set all_afs;
        //////////////////////////////////////////////////

    public:
        aalta_formula(); // new 时调用
        aalta_formula(const aalta_formula& orig); // 拷贝构造函数
        aalta_formula(int op, aalta_formula *left, aalta_formula *right);
        aalta_formula(int atom_id);
        aalta_formula(const char *input);
        aalta_formula(const ltl_formula *formula, bool is_not = false);
        ~aalta_formula();
        static aalta_formula* add_into_all_afs(const aalta_formula *formula); // used in unique() func
        aalta_formula* unique();
        void build (const ltl_formula *formula, bool is_not = false);
        void build_atom(const char *name, bool is_not = false);
        int get_id_by_name(const char *name);
    
    private:
        static aalta_formula *FALSE_;
        static aalta_formula *TRUE_;
        static aalta_formula *TAIL_;
        static aalta_formula *NTAIL_;

    public:
        static aalta_formula* TRUE();
        static aalta_formula* FALSE();
        static aalta_formula* TAIL();
        static aalta_formula* NTAIL();
    
    private:
        size_t hash_; // hash值
        // added for af_prt_set TYPE identification, _id is set in unique ()
        int id_;
        static int max_id_;
    public:
        // added for afp_set TYPE identification
        bool operator == (const aalta_formula& af) const; 
        aalta_formula& operator = (const aalta_formula& af);
        int oper () const;
        bool is_next() const;
        bool is_literal() const;
        bool is_unary() const;
        std::string to_string () const;

    /* transfer formula to specific NF(normal form) */
    public:
        aalta_formula* add_tail (); //add (/\ !Tail) for all Next formulas/occurences
        /**
         * This function split Next subformula by /\ or \/. i.e. X(a/\ b) -> X a /\ X b
         * It is a necessary preprocessing for SAT-based checking
         * 
         * The above comments are copied from old codes.
         * 
         * TODO: I couldn't understand why `split_next` is necessary!
        */
        aalta_formula *split_next();
    };

    aalta_formula* to_af(const ltl_formula *formula);
} // namespace aalta_formula

#endif	/* AALTA_FORMULA_H */