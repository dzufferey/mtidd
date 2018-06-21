new_http_archive(
    name = "catch2git",
    urls = ["https://github.com/catchorg/Catch2/archive/v2.2.3.tar.gz"],
    sha256 = "45e5e12cc5a98e098b0960d70c0d99b7168b711e85fb947dcd4d68ec3f8b8826",
    build_file_content = """
cc_library(
    name = "catch2",
    hdrs = ["Catch2-2.2.3/single_include/catch.hpp"],
    strip_include_prefix = "Catch2-2.2.3/single_include",
    include_prefix = "catch2",
    visibility = ["//visibility:public"],
)""",
)
