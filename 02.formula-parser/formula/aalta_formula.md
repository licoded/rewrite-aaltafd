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