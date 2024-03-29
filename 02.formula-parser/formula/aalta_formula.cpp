/*
 * File:   aalta_formula.cpp
 * Author: Yongkang Li
 *
 * Created on June 22, 2023, 10:40 AM
 */

#include "formula/aalta_formula.h"
#include "ltlparser/trans.h"
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <vector>

namespace aalta
{
    aalta_formula::aalta_formula()
        : op_(e_undefined),
          left_(nullptr),
          right_(nullptr)
    {
        calc_hash();
    } // new 时调用

    aalta_formula::aalta_formula(const aalta_formula &orig) // 拷贝构造函数
    {
        *this = orig; // 浅拷贝
    }

    aalta_formula::aalta_formula(int atom_id)
        : op_(atom_id), // atom_id == literal_id, don't confused this with af_id
          left_(nullptr),
          right_(nullptr)
    {
        calc_hash();
    }
    aalta_formula::aalta_formula(int op,
                                 aalta_formula *left,
                                 aalta_formula *right)
        : op_(op),
          left_(left),
          right_(right)
    {
        calc_hash();
    }

    aalta_formula::aalta_formula(const char *input)
    {
        /**
         * ERROR: assignment to 'this'
         * CODE: this = new aalta_formula(getAST(input), false);
         */
        // *this = aalta_formula(getAST(input), false);
        build(getAST(input), false); // 这样少一次 ctor 创建对象的消耗
        calc_hash();
    }

    aalta_formula::aalta_formula(const ltl_formula *formula, bool is_not)
    // TODO: variable `is_not`, I want to change a more clear/articulate name
    {
        build(formula, is_not);
        calc_hash();
    }

    aalta_formula::~aalta_formula() {}

    /**
     * a static method
     *  - used in unique() func
     * TODO: I think that `unique()` and `all_afs` can be extracted into a extra class
     */
    aalta_formula *aalta_formula::add_into_all_afs(const aalta_formula *af)
    {
        aalta_formula *new_unique_ptr = new aalta_formula(*af); // 对应旧的 clone 函数
        aalta_formula::all_afs.insert(new_unique_ptr);
        new_unique_ptr->id_ = max_id_++;
        new_unique_ptr->unique_ = new_unique_ptr;
        aalta_formula::id_to_af.insert({new_unique_ptr->id_, new_unique_ptr});
        aalta_formula::id_to_afs.insert({new_unique_ptr->id_, new_unique_ptr->to_string()});
        return new_unique_ptr;
    }

    aalta_formula *aalta_formula::unique()
    {
        if (unique_ != NULL)
            return unique_;
        this->af_s_ = this->to_string();
        afp_set::const_iterator iter = all_afs.find(this);
        unique_ = (iter != all_afs.end())
                      ? (*iter)
                      : aalta_formula::add_into_all_afs(this);
        unique_->af_s_ = unique_->to_string();
        return unique_;
    }

    aalta_formula *aalta_formula::simplify()
    {
        if (simp_ != NULL)
            return simp_;

        switch (op_)
        {
        // case e_and: // &
        //     simp_ = aalta_formula::simplify_and(left_, right_);
        //     break;
        // case e_or: // |
        //     simp_ = aalta_formula::simplify_or(left_, right_);
        //     break;
        // case e_next: // X
        //     simp_ = aalta_formula::simplify_next(right_);
        //     break;
        case e_until: // U
            simp_ = simplify_until(left_, right_);
            break;
        case e_release: // R
            simp_ = simplify_release(left_, right_);
            break;
        case e_not: // ! 只会出现在原子前，因此可不做处理
                    // break;
        default:    // atom
            simp_ = unique();
            break;
        }

        /* TOOD: the below line may could be optimized */
        simp_->unique_ = simp_->simp_ = simp_;
        return simp_;
    }

    /**
     * NOTE: a U b
     *  - `a U b` only requires `F(b)` instead of `F(G(b))`, i.e. b only needs to be true at a time point
     *  - G(b) is enough, so it could start with b is true, a could be false forever
     *  - F(b) must be true, i.e. b must be true at some future time point
     * NOTE: don't forget to check simp cannot be nullptr
    */
    aalta_formula *aalta_formula::simplify_until(aalta_formula *l, aalta_formula *r)
    {
        aalta_formula *l_simp = l->simplify();
        aalta_formula *r_simp = r->simplify();
        aalta_formula *simp;

        if (false
            || l_simp->oper() == e_false // false U b = b, as b must be true at current time point, and it's enough to make `false U b` to be true
            || r_simp->oper() == e_false // a U false = false, as F(b) must be true and F(b) === F(false) === false now
            || r_simp->oper() == e_true  // a U true = true, as G(b) is enough and G(b) === G(true) === true now
        )
            simp = r_simp;

        else
            simp = aalta_formula(e_until, l_simp, r_simp).unique();

        return simp;
    }

    /**
     * NOTE: a R b
     *  - both a and b is required at some time point
     *  - b must be true at first/before released
     *  - G(b) is enough, so a can always be false, as only as b is true forever
     * NOTE: don't forget to check simp cannot be nullptr
    */
    aalta_formula *aalta_formula::simplify_release(aalta_formula *l, aalta_formula *r)
    {
        aalta_formula *l_simp = l->simplify();
        aalta_formula *r_simp = r->simplify();
        aalta_formula *simp;

        if (false 
            //l_simp->oper() == e_false    // false R b = G(b) != b !!!, as b can never be released in this case, since a === false
            || l_simp->oper() == e_true  // true R b = b, as b must be true at current time point (because b must be true at first/before released), and it's enough to make `true R b` to be true 
            || r_simp->oper() == e_false // a R false = false, as false must be true at first/before release, and it's enought to judge `a R false` is false
            || r_simp->oper() == e_true  // a R true = true, as G(b) is enough, and G(b) === G(true) === true now
        )
            simp = r_simp;

        // Both `a R !a` and `!a R a` are equivalent to `false R right_formula` (i.e. G(right_formula))
        else if (false
            || (l_simp->oper() == e_not && l_simp->right_ == r_simp)    // !a R a  === G( a) === false R  a, as  a can never be released, since the released time point required `!a & a`
            || (r_simp->oper() == e_not && r_simp->right_ == l_simp)    //  a R !a === G(!a) === false R !a, as !a can never be released, since the released time point required ` a & !a`
        )
            simp = aalta_formula(e_release, FALSE(), r_simp).simplify();
        
        else
            simp = aalta_formula(e_release, l_simp, r_simp).unique();

        return simp;
    }

    /* I think `simplify_and()` func is meaningless, at least not so important, as SAT can handle this. */
    // aalta_formula *aalta_formula::simplify_and(aalta_formula *l, aalta_formula *r)
    // {
    //     aalta_formula *l_simp = l->simplify();
    //     aalta_formula *r_simp = r->simplify();

    //     /* This case ( true & true ) is included in the below two cases */
    //     // if(l_simp->oper() == e_true && r_simp->oper() == e_true)
    //     //     return TRUE();
    //     if(l_simp->oper() == e_true)
    //         return r_simp;
    //     if(r_simp->oper() == e_true)
    //         return l_simp;
    //     if(l_simp->oper() == e_false || r_simp->oper() == e_false)
    //         return FALSE();
    // }

    /**
     * 将ltl_formula转成aalta_formula结构，
     * 并处理！运算，使其只会出现在原子前
     * @param formula
     * @param is_not 标记此公式前是否有！
     *
     * TODO:
     *  - build 函数中, 使用 `aalta_formula().unique()` 方法会创建2~3次对象, 想办法提高效率减少对象创建次数
     *      - 比如, 用一个函数解决此问题
     */
    void
    aalta_formula::build(const ltl_formula *formula, bool is_not)
    {
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
            build_atom(formula->_var, is_not);
            break;
        case eNOT:
            build(formula->_right, is_not ^ 1);
            break;
        case eNEXT:      // Xa -- [!(Xa) = N(!a) = Tail | X(!a)]
            if (!is_not) // Xa
            {
                op_ = e_next;
                right_ = aalta_formula(formula->_right, is_not).unique();
            }
            else // N(!a)
            {
                ltl_formula *not_a = create_operation(eNOT, NULL, formula->_right);
                ltl_formula *N_not_a = create_operation(eWNEXT, NULL, not_a);
                build(N_not_a, false);
                destroy_node(not_a);
                destroy_node(N_not_a);
            }
            break;
        case eWNEXT:     // [Na = Tail | Xa ] -- [!(Na) = X(!a)]
            if (!is_not) // Tail | Xa
            {
                ltl_formula *Xa = create_operation(eNEXT, NULL, formula->_right);
                *this = *(aalta_formula(e_or, TAIL(), to_af(Xa)).unique());
                destroy_node(Xa);
            }
            else // X(!a)
            {
                ltl_formula *not_a = create_operation(eNOT, NULL, formula->_right);
                ltl_formula *X_not_a = create_operation(eNEXT, NULL, not_a);
                build(X_not_a, false);
                destroy_node(not_a);
                destroy_node(X_not_a);
            }
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
        {
            ltl_formula *Ga = create_operation(eGLOBALLY, NULL, formula->_left);
            ltl_formula *aUb = create_operation(eUNTIL, formula->_left, formula->_right);
            ltl_formula *now = create_operation(eOR, Ga, aUb);
            *this = *(aalta_formula(now, is_not).unique());
            destroy_node(Ga);
            destroy_node(aUb);
            break;
        }
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
        case eIMPLIES: // a->b = !a|b
        {
            ltl_formula *not_a = create_operation(eNOT, NULL, formula->_left);
            ltl_formula *now = create_operation(eOR, not_a, formula->_right);
            *this = *(aalta_formula(now, is_not).unique());
            destroy_node(not_a);
            destroy_node(now);
            break;
        }
        case eEQUIV: // a<->b = (a->b)&(b->a)
        {
            ltl_formula *imply_left = create_operation(eIMPLIES, formula->_left, formula->_right);
            ltl_formula *imply_right = create_operation(eIMPLIES, formula->_right, formula->_left);
            ltl_formula *now = create_operation(eAND, imply_left, imply_right);
            *this = *(aalta_formula(now, is_not).unique());
            destroy_node(imply_left);
            destroy_node(imply_right);
            destroy_node(now);
            // 为什么 destroy 顺序是这样的? 如果顺序错了会有影响吗?
            break;
        }
        default:
            // print_error("the formula cannot be recognized by aalta!");
            // may be:
            //  - type/input error syntax formula
            //  - need add codes for new/custom defined operator
            exit(1);
            break;
        }
    }

    /* 初始化非静态成员变量 */
    /* 初始化静态成员变量 */
    std::vector<std::string> aalta_formula::names = {
        "true", "false", "Literal", "!", "|", "&", "X", "N", "U", "R", "Undefined"}; // 存储操作符的名称以及原子变量的名称
    std::unordered_map<std::string, int> aalta_formula::name_id_map;                 // 名称和对应的位置映射
    int aalta_formula::max_id_ = 1;
    aalta_formula::afp_set aalta_formula::all_afs;
    std::map<int, aalta_formula *> aalta_formula::id_to_af;
    std::map<int, std::string> aalta_formula::id_to_afs;
    aalta_formula *aalta_formula::TRUE_ = nullptr;
    aalta_formula *aalta_formula::FALSE_ = nullptr;
    aalta_formula *aalta_formula::TAIL_ = nullptr;
    aalta_formula *aalta_formula::NTAIL_ = nullptr;

    aalta_formula *aalta_formula::TRUE()
    {
        if (TRUE_ == nullptr)
            TRUE_ = aalta_formula(e_true).unique();
        return TRUE_;
    }
    aalta_formula *aalta_formula::FALSE()
    {
        if (FALSE_ == nullptr)
            FALSE_ = aalta_formula(e_false).unique();
        return FALSE_;
    }
    aalta_formula *aalta_formula::TAIL()
    {
        if (TAIL_ == nullptr)
        {
            auto tail_s_arr = {"tail", "Tail", "TAIL"};
            const int tail_id = aalta_formula::get_id_by_names(tail_s_arr);
            TAIL_ = aalta_formula(tail_id).unique();
        }
        return TAIL_;
    }
    aalta_formula *aalta_formula::NTAIL()
    {
        if (NTAIL_ == nullptr)
            NTAIL_ = aalta_formula(e_not, NULL, TAIL()).unique();
        return NTAIL_;
    }

    /**
     * 计算hash值
     */
    inline void
    aalta_formula::calc_hash()
    {
        hash_ = 1315423911; // HASH_INIT;
        hash_ = (hash_ << 5) ^ (hash_ >> 27) ^ op_;

        if (left_ != NULL)
            hash_ = (hash_ << 5) ^ (hash_ >> 27) ^ left_->hash_;
        if (right_ != NULL)
            hash_ = (hash_ << 5) ^ (hash_ >> 27) ^ right_->hash_;

        hash_ = (hash_ << 5) ^ (hash_ >> 27);
    }

    /**
     * 重载等于符号, 用于 hashset -- afp_set TYPE 的判断相等的函数
     * @param af
     * @return
     */
    bool aalta_formula::operator==(const aalta_formula &af) const
    {
        return left_ == af.left_ && right_ == af.right_ && op_ == af.op_; // && tag_ == af.tag_;
    }

    /**
     * 重载赋值操作符, 用于拷贝构造函数, unique 函数
     * @param af
     * @return
     */
    aalta_formula &aalta_formula::operator=(const aalta_formula &af)
    // TODO:
    //  - 这样(按照引用而非对象)应该是为了减少拷贝的开销
    //  - 如果按照对象而非引用去定义, 会怎样, 有什么意义/适用情况吗?
    //  - 网上查到的: 为什么要返回引用而不是对象?
    {
        if (this != &af)
        {
            this->left_ = af.left_;
            this->right_ = af.right_;
            // this->tag_ = af.tag_;
            this->op_ = af.op_;
            this->hash_ = af.hash_;
            // this->length_ = af.length_;
            this->id_ = af.id_;
            this->unique_ = af.unique_;
            // this->simp_ = af.simp_;
        }
        return *this;
    }

    /**
     * 获取op操作符
     * @return
     */
    int aalta_formula::oper() const
    {
        return op_;
    }

    bool aalta_formula::is_next() const
    {
        return oper() == e_next;
    }

    /**
     * Used in aalta::Solver.
     */
    bool aalta_formula::is_label() const
    {
        return oper() == e_not || oper() > e_undefined;
    }

    /**
     * literals, e.g. a/b/c/aaa
     * include true and false
     */
    bool aalta_formula::is_literal() const
    {
        // I want to implement this function like below, can I do it?
        // > I think it's impossible. Because the build_atom function doesn't use `e_literal`;
        // > It just use id instead.
        // return oper() == e_literal;
        return left_ == nullptr && right_ == nullptr;
        /**
         * OR
         *  - `oper() > e_undefined` 但是这样判断会漏掉 true 和 false
         */
    }
    bool aalta_formula::is_unary() const
    {
        return left_ == nullptr;
    }

    std::string aalta_formula::to_string() const
    {
        if (is_literal())
            return aalta_formula::names[oper()];

        std::string inner_s;
        std::string operator_s = aalta_formula::names[oper()];

        if (is_unary())
            inner_s = operator_s + " " + right_->to_string();
        else
            inner_s = left_->to_string() + " " + operator_s + " " + right_->to_string();

        return "(" + inner_s + ")";
    }

    // to_s_string
    std::string aalta_formula::to_set_string()
    {
        return aalta_formula::to_set_string(to_set());
    }

    // to_s_string
    std::string aalta_formula::to_set_string(const af_prt_set &af_prt_set_)
    {
        std::string s;
        for (auto label_af : af_prt_set_)
        {
            if (s != "")
                s += ", ";
            s += label_af->to_string();
        }
        return s;
    }

    /**
     * add (/\ !Tail) for all Next formulas/occurences
     *  - 在所有出现 Next(X) 的位置, 添加 /\ !Tail
     *  - TODO: why not just add /\ !Tail in the outer level?
     */
    aalta_formula *aalta_formula::add_tail()
    {
        if (this == nullptr) // I'm not sure about the correctness of this.
            return nullptr;
        aalta_formula *res = nullptr;
        if (is_next())
        {
            aalta_formula *new_next = aalta_formula(e_next, nullptr, right_->add_tail()).unique();
            res = aalta_formula(e_and, NTAIL(), new_next).unique();
        }
        else
            res = aalta_formula(oper(), left_->add_tail(), right_->add_tail()).unique();
        return res;
    }

    aalta_formula *aalta_formula::split_next()
    {
        if (this == nullptr) // I'm not sure about the correctness of this.
            return nullptr;
        if (is_literal())
            return this;

        aalta_formula *res;
        if (oper() == e_next)
        {
            if (right_->oper() == e_and || right_->oper() == e_or)
            // e.g. X(a & b) = X(a) & X(b)
            {
                aalta_formula *left_next = aalta_formula(oper(), NULL, right_->left_).unique();
                aalta_formula *right_next = aalta_formula(oper(), NULL, right_->right_).unique();
                res = aalta_formula(right_->oper(), left_next->split_next(), right_next->split_next()).unique();
            }
            else
            {
                res = aalta_formula(oper(), NULL, right_->split_next()).unique();
                if (right_->oper() == e_and || right_->oper() == e_or)
                {
                    // TODO: I couldn't understand the following code
                    //       give me an example!
                    res = res->split_next();
                }
            }
        }
        else
            res = aalta_formula(oper(), left_->split_next(), right_->split_next()).unique();
        return res;
    }

    aalta_formula *to_af(const ltl_formula *formula)
    {
        return aalta_formula(formula, false).unique();
    }

    /**
     * TODO: merge it to the below func `to_set()`, recursion -> loop?
     */
    void aalta_formula::to_set(af_prt_set &result)
    {
        if (oper() != e_and)
            result.insert(this);
        else
        {
            left_->to_set(result);
            right_->to_set(result);
        }
    }

    aalta_formula::af_prt_set aalta_formula::to_set()
    {
        af_prt_set result;
        to_set(result);
        return result;
    }

    // TODO: understand it and optimize
    aalta_formula::af_prt_set aalta_formula::to_or_set()
    {
        af_prt_set res;
        if (oper() != e_or)
            res.insert(this);
        else
        {
            af_prt_set tmp = left_->to_or_set();
            res.insert(tmp.begin(), tmp.end());
            tmp = right_->to_or_set();
            res.insert(tmp.begin(), tmp.end());
        }
        return res;
    }

    /**
     * @param &ands vector<af *>
     * @return \/ (ands[i]) or TRUE (if ands is empty)
     */
    aalta_formula *formula_from(std::vector<aalta_formula *> &ands)
    {
        if (ands.empty())
            return aalta_formula::TRUE();
        aalta_formula *res = NULL;
        for (std::vector<aalta_formula *>::iterator it = ands.begin(); it != ands.end(); it++)
        {
            if (res == NULL) // just to proceed the first item of ands
                res = *it;
            else
                res = aalta_formula(e_and, res, *it).unique();
        }
        return res;
    }
} // namespace aalta_formula