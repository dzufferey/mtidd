load("//tools:mtidd.bzl", "mtidd_test")

# to simplify the import, see https://stackoverflow.com/questions/53306147/include-file-downloaded-by-bazel-http-file
cc_library(
  name = "catch",
  hdrs = ["@catch2//file"],
  visibility = ["//test:__pkg__"],
  strip_include_prefix = "/file",
  include_prefix = "catch2",
  linkstatic = True,
)

mtidd_test(
    name = "internalizer_tests",
)

mtidd_test(
    name = "interval_tests",
)

mtidd_test(
    name = "lattice_tests",
)

mtidd_test(
    name = "cache_tests",
)

mtidd_test(
    name = "mtidd_tests",
)

mtidd_test(
    name = "partition_iterator_tests",
)

mtidd_test(
    name = "partition_tests",
)
