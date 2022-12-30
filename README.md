# MTIDD

A C++ library for multi-terminal (reduced ordered) interval decision diagrams.

## Compiling

You need to install [bazel](https://bazel.build) and have a modern C++ compiler.

With [Bazelisk](https://github.com/bazelbuild/bazelisk) you can build an run with:
```sh
bazelisk build //...
bazelisk test //...
```

## Status

Started to work on it.
Nothing concrete yet.

#### Design Choices

- The code is parametric on the type of variables and terminal elements.
- The terminal elements must form a [lattice](mtidd/lattice.h).
- The variable ordering is implicitly given by an [internalizer](mtidd/internalizer.h) which maps each variable to a unique index.
  The order on the indices becomes the variable order.

#### ToDo

- reordering `void reorder(std::map<int, int> const & var_idx_bijection) { ... }`
   * swap operation for two variables adjacent in the ordering
   * partial lookup for two steps in the variable order (needed in the swap)
   * reordering is like bubble sort with the new order and using swap
- implement proper reference counting to manage the memory
   * use c++ [smart pointer](https://en.cppreference.com/w/cpp/memory/shared_ptr) ?
- map operation to transform the type of variables and terminal
