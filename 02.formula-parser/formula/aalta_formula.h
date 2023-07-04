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
#include <map>
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
        typedef std::unordered_set<aalta_formula *, af_prt_hash> af_prt_set;

    private:
        ////////////
        //成员变量//
        //////////////////////////////////////////////////
        int op_; // 操作符or操作数(原子atom)
        aalta_formula *left_ = nullptr; // 操作符左端公式
        aalta_formula *right_ = nullptr; // 操作符右端公式
        // int length_; //公式长度
        aalta_formula *unique_ = nullptr; // 指向唯一指针标识
        std::string af_s_;
        // aalta_formula *simp_ = nullptr; // 指向化简后的公式指针
        static std::vector<std::string> names; // 存储操作符的名称以及原子变量的名称
        static std::unordered_map<std::string, int> name_id_map; // 名称和对应的位置映射
        static afp_set all_afs;
        static std::map<int, aalta_formula *> id_to_af;
        static std::map<int, std::string> id_to_afs;
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
        static int get_id_by_name(const char *name);
        static int get_id_by_names(const std::vector<const char *> &name_arr);

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
        static int max_id_; // id count for af ptr
        void calc_hash();
    public:
        // added for afp_set TYPE identification
        bool operator == (const aalta_formula& af) const; 
        aalta_formula& operator = (const aalta_formula& af);
        int oper () const;
        bool is_next() const;
        bool is_label() const;
        bool is_literal() const;
        bool is_unary() const;
        inline bool is_tf() const;          // used in `Solver`
        inline bool is_U_or_R() const;      // used in `Solver`
        inline bool is_and_or_or() const;      // used in `Solver`
        inline bool is_globally() const;    // used in `Solver`
        inline bool is_wider_globally() const;    // used in `Solver`
        inline bool is_future() const;    // used in `Solver`
        std::string to_string () const;
        inline int id() { return id_; }
        inline aalta_formula* l_af() { return left_; }
        inline aalta_formula* r_af() { return right_; }
        inline int r_id() { return right_->id_; } // used in `Solver`
        inline int l_id() { return left_->id_; } // used in `Solver`

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
        void to_set(af_prt_set &result);    // TODO: merge it to the below func `to_set()`, recursion -> loop?
        af_prt_set to_set();                // used in `Solver::block_formula()`
        af_prt_set to_or_set();                // used in `Solver::block_formula()`
    };

    aalta_formula* to_af(const ltl_formula *formula);
    aalta_formula* formula_from(std::vector<aalta_formula *> &ands);

    inline int
    aalta_formula::get_id_by_name(const char *name)
    {
        int id; // NOTE: this is operator id, not af id
        const auto &it = name_id_map.find(name);
        if (it == name_id_map.end())
        { // 此变量名未出现过，添加之
            id = names.size();
            name_id_map.insert({name, id});
            names.push_back(name);
        }
        else
            id = it->second;
        return id;
    }

    /**
     * bind all elems in `name_arr` with `id = names.size()`, and just/only push the first elem into `names` for `af->to_string()` func
     * NOTE: 
     *  - `id = names.size()` instead of `++max_id_`
     *  - there is no `max_id_` for names
     *  - the `max_id_` variable is for (unique) af ptr
    */
    inline int
    aalta_formula::get_id_by_names(const std::vector<const char *> &name_arr)
    {
        // TODO: check if some names in `vector<const char*> names` already exist in `aalta_formula::names`
        const int id = names.size();
        names.push_back(name_arr.front());
        for (auto name : name_arr)
            name_id_map.insert({name, id});
        return id;
    }

    /**
     * 添加原子变量
     * @param name
     * @param is_not -- 该变量是有用到的, build函数中调用时可能传入true也可能传入false
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

    // is true or false
    inline bool aalta_formula::is_tf() const
    {
        return oper() == e_true || oper() == e_false;
    }

    // is true or false
    inline bool aalta_formula::is_U_or_R() const
    {
        return oper() == e_until || oper() == e_release;
    }

    // is and or or
    inline bool aalta_formula::is_and_or_or() const
    {
        return oper() == e_and || oper() == e_or;
    }

    // check whether it's a G formula
    inline bool aalta_formula::is_globally() const
    {
        /**
         * G(a) = false R a
         *  - 'check `oper() = R` firstly' is very important!
         */
        return oper() == e_release && left_->oper() == e_false;
    }

    // include more cases than `is_globally()` func
    // e.g. G(a) & G(b), G(a) | G(b)
    inline bool aalta_formula::is_wider_globally() const
    {
        if(is_globally())
            return true;
        if(is_and_or_or())
            return left_->is_wider_globally() && right_->is_wider_globally();
        return false;
    }

    inline bool aalta_formula::is_future() const
    {
        /**
         * F(a) = true U a
         *  - 'check `oper() = U` firstly' is very important!
         */
        return oper() == e_until && left_->oper() == e_true;
    }
} // namespace aalta_formula

#endif	/* AALTA_FORMULA_H */