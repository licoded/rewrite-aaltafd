/* 
 * File:   aalta_formula.cpp
 * Author: Yongkang Li
 *
 * Created on June 22, 2023, 10:40 AM
*/

#include "aalta_formula.h"
#include "ltlparser/trans.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace aalta {
    aalta_formula::aalta_formula()
        : op_(e_undefined),
          left_(nullptr),
          right_(nullptr)
        {} // new 时调用

    aalta_formula::aalta_formula(const aalta_formula &orig) // 拷贝构造函数
    {
        *this = orig; // 浅拷贝
    }

    aalta_formula::aalta_formula(int atom_id)
        : op_(atom_id), // atom_id == literal_id, don't confused this with af_id
          left_(nullptr),
          right_(nullptr)
        {}
    aalta_formula::aalta_formula(int op,
                                 aalta_formula *left,
                                 aalta_formula *right)
        : op_(op),
          left_(left),
          right_(right) {}

    aalta_formula::aalta_formula(const char *input)
    {
        /**
         * ERROR: assignment to 'this'
         * CODE: this = new aalta_formula(getAST(input), false);
        */
        // *this = aalta_formula(getAST(input), false);
        build(getAST(input), false); // 这样少一次 ctor 创建对象的消耗
    }

    aalta_formula::aalta_formula(const ltl_formula *formula, bool is_not)
    // TODO: variable `is_not`, I want to change a more clear/articulate name
    {
        build(formula, is_not);
    }

    aalta_formula::~aalta_formula() {}

    /**
     * a static method
     *  - used in unique() func
     * TODO: I think that `unique()` and `all_afs` can be extracted into a extra class
    */
    aalta_formula* aalta_formula::add_into_all_afs(const aalta_formula *af)
    {
        aalta_formula* new_unique_ptr = new aalta_formula(*af); // 对应旧的 clone 函数
        aalta_formula::all_afs.insert(new_unique_ptr);
        new_unique_ptr->id_ = max_id_++;
        new_unique_ptr->unique_ = new_unique_ptr;
        return new_unique_ptr;
    }

    aalta_formula* aalta_formula::unique()
    {
        if(unique_ != NULL)
            return unique_;
        afp_set::const_iterator iter = all_afs.find(this);
        // TODO: how to understand `const_iterator` and the following code `unique_ = *iter`
        unique_ = (iter != all_afs.end()) 
            ? (*iter) 
            : aalta_formula::add_into_all_afs(this);
        return unique_;
    }

    inline int
    aalta_formula::get_id_by_name(const char *name)
    {
        int id; // NOTE: this is operator id, not af id
        const auto& it = name_id_map.find(name);
        if (it == name_id_map.end())
        { // 此变量名未出现过，添加之
            id = names.size();
            name_id_map.insert(std::make_pair(name, id));
            names.push_back(name);
        }
        else
            id = it->second;
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
                build_atom (formula->_var, is_not);
                break;
            case eNOT:
                build(formula->_right, is_not^1);
                break;
            case eNEXT: // Xa -- [!(Xa) = N(!a)]
                op_ = is_not ? e_w_next : e_next;
                right_ = aalta_formula(formula->_right, is_not).unique();
                break;
            case eWNEXT: // Na -- [!(Na) = X(!a)]
                op_ = is_not ? e_next : e_w_next;
                right_ = aalta_formula(formula->_right, is_not).unique();
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
    /**
     * ERROR: a nonstatic data member may not be defined outside its class
     * CODE: int aalta_formula::op_ = e_undefined; // 操作符or操作数(原子atom)
    */
    /* 初始化静态成员变量 */
    // TODO: 能否在 define 时保留 static 声明, like `static int a = 0;`
    // note: 即使不赋初值, 也必须 define, 不然静态变量不存在, 会报错 'identifier "XXX" is undefined'
    //       > TODO: 不知道非静态变量是不是也是这样?
    std::vector<std::string> aalta_formula::names = {
        "true", "false", "Literal", "!", "|", "&", "X", "N", "U", "R", "Undefined"
    }; // 存储操作符的名称以及原子变量的名称
    std::unordered_map<std::string, int> aalta_formula::name_id_map; // 名称和对应的位置映射
    int aalta_formula::max_id_ = 1;
    aalta_formula::afp_set aalta_formula::all_afs;
    // can't declare and define non-const static variables in the same time
    aalta_formula* aalta_formula::TRUE_ = nullptr;
    aalta_formula* aalta_formula::FALSE_ = nullptr;
    aalta_formula* aalta_formula::TAIL_ = nullptr;
    aalta_formula* aalta_formula::NTAIL_ = nullptr;

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
    aalta_formula* aalta_formula::TAIL()
    {
        if (TAIL_ == nullptr)
            TAIL_ = aalta_formula("Tail").unique();
        return TAIL_;
    }
    aalta_formula* aalta_formula::NTAIL()
    {
        if (NTAIL_ == nullptr)
            NTAIL_ = aalta_formula (e_not, NULL, TAIL()).unique ();
        return NTAIL_;
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
    //  - 判断对象相等时, 应该会走这里吧?
    //  - 这样应该是为了减少拷贝的开销
    //  - 如果按照对象而非引用去定义, 会怎样, 有什么意义/适用情况吗?
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

    bool aalta_formula::is_literal() const
    {
        // I want to implement this function like below, can I do it?
        // > I think it's impossible. Because the build_atom function doesn't use `e_literal`;
        // > It just use id instead.
        // return oper() == e_literal;
        return left_ == nullptr && right_ == nullptr;
    }
    bool aalta_formula::is_unary() const
    {
        return left_ == nullptr;
    }

    std::string aalta_formula::to_string () const
    {
        if(is_literal())
            /**
             * TODO: why use `,` instead of `;` will lead to ERROR, which can be run but unexpected result.
             *      std::string literal_s = aalta_formula::names[oper()], inner_s = literal_s;
            */
            return aalta_formula::names[oper()];

        std::string inner_s;
        std::string operator_s = aalta_formula::names[oper()];

        if (is_unary())
            inner_s = operator_s + " " + right_->to_string();
        else
            inner_s = left_->to_string() + " " + operator_s + " " + right_->to_string();

        return "(" + inner_s + ")";
    }


    /**
     * add (/\ !Tail) for all Next formulas/occurences
     *  - 在所有出现 Next(X) 的位置, 添加 /\ !Tail
     *  - TODO: why not just add /\ !Tail in the outer level?
    */
    aalta_formula *aalta_formula::add_tail()
    {
        if(this == nullptr) // I'm not sure about the correctness of this.
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
} // namespace aalta_formula