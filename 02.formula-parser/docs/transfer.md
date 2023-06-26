```cpp
af = af->nnf ();                // 构造函数, 调用 build 时已经保证了, build 函数中 `!` 会一直向下传递
af = af->add_tail ();
af = af->remove_wnext ();
af = af->simplify ();
af = af->split_next ();
```

- `af->add_tail()`
- `af->remove_wnext()`
- `af->simplify()`
    - 我认为该函数, 目的是简化公式以降低 SAT 求解的复杂度
    - 因此, 该函数并不是必须的, 先跳过!
- `af->split_next()`