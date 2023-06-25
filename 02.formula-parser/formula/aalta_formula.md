## 报错记录

### "multiple definition of `XXX`, first defined here"

**参考链接:**
- [【C++】解决C++ “multiple definition of .. first defined here“问题 -- CSDN](https://blog.csdn.net/qq_44886213/article/details/124586164)

**我的理解:**
- 即使有 `ifndef` 保护, 由于不同的 `.cpp` 文件被分别编译为 `.o` 文件, `.h` 还是会被多次 include
- 而重复 declaration 不会报错, 但重复的 definition 则会报错
- 所以, 参考 `aaltafd` 源码, 解决办法之一就是仅在 `.h` 对应的 `.cpp` 中 define, 在 `.h` 中只 declare
- 目前有两个疑问:
    - 为什么 `.h` 中可以放 struct 的 definition? 至少我认为那是 definition, 而不是 declaration
    - 这样并没有从根本上解决问题, `ifndef` 的好处是啥? 有啥更好的解决办法?

## 风格指南

### 类内成员变量命名

> 最终决定: 才用后置下划线

- 使用下划线的原因
    - 防止重名, 从而方便构造函数初始化
- 不采用前置的原因
    - 方便智能提示快速匹配+搜索

## 重要的理解

- `Oper() <= Undefined` 不是一定成立的吗? 这样做判断的意义何在?
    - `Oper()` 不仅可能是操作符, 还可以是操作数!

## 优化想法

- 基类(very basic)
    - 提供 `==`, `=` 重载
    - `oper()`, `is_next()` 方法
- unique 类 -- (最高级)
    - 实现 unique 相关的
    - 包括声明 all_afs
- transfer类
    - 负责完成后续转换

> 感觉父子类关系不合适, 要用组合/委托来实现!

## 遇到的问题

### `,` 语法

why use `,` instead of `;` will lead to ERROR, which can be run but unexpected result.

```cpp
std::string literal_s = aalta_formula::names[oper()], inner_s = literal_s;
```

after this code execution, `literal_s` is not equal to `inner_s` -- `inner_s` is empty(`""`), while `literal_s` is as expected

> I have worked out this; It's just because that I have redefined `inner_s` in if body, which override the outer definition of `inner_s`.
> So, it has nothing about the `,`(comma) syntax;

### 类的静态成员变量

- 即使不赋初值, 也必须 define
    - 不然静态变量不存在, 会报错 'identifier "XXX" is undefined'
- 非静态成员变量不能在类外定义/初始化
    - 报错 'a nonstatic data member may not be defined outside its class'

### 常成员函数

- `type func(/* args */) const { /* func body */}`
    - 注意 const 的位置
- 在常成员函数中, 不能修改 (non-static) member variables, 但不关心 static member variables
    - 经过测试, 可以修改 static member variables

### `pointer to const` and `const pointer`

> refer link: https://github.com/licoded/self-study-drafts/issues/116

- pointer to const / 常量指针
    - `const int *p = &a;`
- const pointer / 常指针 / 指针常量
    - `int * const p = &a;`

```cpp
afp_set::const_iterator iter = all_afs.find(this);
// const_iterator is 'pointer to const', the following line is proof -- as it can be re-assign by a new value
iter = all_afs.find(this);
```

### `char *` 强制类型转换成 `string` 会有警告

```cpp
// 不加 const 前缀会报警告如下:
// warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
std::vector<const char *> str = {
    "a",
    "!a",
    "X(a)",
    "X(!a)",
    "!X(a)",
    "X(a|b) & c",
    "X(a|b) & X(c)",
    "X(a|b) & G(X(c))",
};
```

### 注意多个字符串, 只换行没有`,`会自动拼接在一起

> !!!