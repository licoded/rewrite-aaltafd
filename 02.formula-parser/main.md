## 对旧代码的质疑

- `af = af->add_tail();` 应该放在最后的
    - 但它应该放在 `af = af->simplify();` 之前