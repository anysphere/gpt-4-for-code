load("@rules_rust//rust:defs.bzl", "rust_library", "rust_test")

rust_library(
    name = "example",
    srcs = glob(["src/**/*.rs"]),
    rustc_flags = [
        # we allow unused imports, because they are annoying to avoid while developing
        # and we don't want people to ignore warnings! which happens if there are too many warnings
        "-Aunused_imports",
        # we disallow unused results, because they are almost always bugs
        "-Dunused_must_use",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@global_crate_index//:tree-sitter",
        "@tree_sitter_python",
        "@tree_sitter_rust",
        "@tree_sitter_go",
        "@tree_sitter_javascript",
        "@tree_sitter_typescript",
    ],
)

rust_test(
    name = "test",
    crate = ":example",
)
