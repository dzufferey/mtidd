load("//tools:mtidd.bzl", "mtidd_test")

cc_library(
    name = "catch2",
    hdrs = ["@catch2Download//file"],
    visibility = ["//visibility:private"],
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
    name = "mtidd_tests",
)

mtidd_test(
    name = "partition_iterator_tests",
)

mtidd_test(
    name = "partition_tests",
)
