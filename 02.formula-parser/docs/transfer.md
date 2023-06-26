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
- `af->split_next()`