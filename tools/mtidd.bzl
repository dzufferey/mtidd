
def mtidd_test(
        name,
        size = None,
        srcs = None,
        **kwargs):
    if size == None:
        size = "small"
    if srcs == None:
        srcs = ["%s.cpp" % name]
    native.cc_test(
        name = name,
        size = size,
        srcs = srcs,
        deps = [
            "//mtidd:mtidd",
            "//test:catch"
        ],
        **kwargs
    )
