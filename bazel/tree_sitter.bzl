"""
GPT-4-generated! A few iterations of (1) instruction and (2) small follow-ups and (3) letting GPT-4 look at the Bazel error messages and fix them.
"""

def _tree_sitter_lang_impl_with_file(repository_ctx, lang, file):
  # Get the http archive from the tree-sitter repo
  repository_ctx.download_and_extract(
    url = "https://github.com/tree-sitter/tree-sitter-" + lang + "/archive/master.zip",
    stripPrefix = "tree-sitter-" + lang + "-master",
    type = "zip",
    # output = "//third_party/tree_sitter_" + lang,
  )
  
  repository_ctx.file(
    "WORKSPACE",
    content = "",
  )

  # Build the cc_lib from src/parser.c and src/scanner.c
  repository_ctx.file(
    "BUILD",
    content = file.format(lang = lang),
  )


TREE_SITTER_BUILD_FILE_WITH_CC = """cc_library(
  name = "tree_sitter_{lang}",
  srcs = ["src/parser.c", "src/scanner.cc"],
  copts = ["-Iexternal/tree_sitter_{lang}/src"],
  hdrs = ["src/tree_sitter/parser.h"],
  linkstatic = True,
  visibility = ["//visibility:public"],
)
"""

def _tree_sitter_lang_impl_with_cc(repository_ctx, lang):
  _tree_sitter_lang_impl_with_file(repository_ctx, lang, TREE_SITTER_BUILD_FILE_WITH_CC)

TREE_SITTER_BUILD_FILE_WITH_C = """cc_library(
  name = "tree_sitter_{lang}",
  srcs = ["src/parser.c", "src/scanner.c"],
  copts = ["-Iexternal/tree_sitter_{lang}/src"],
  hdrs = ["src/tree_sitter/parser.h"],
  linkstatic = True,
  visibility = ["//visibility:public"],
)
"""

def _tree_sitter_lang_impl_with_c(repository_ctx, lang):
  _tree_sitter_lang_impl_with_file(repository_ctx, lang, TREE_SITTER_BUILD_FILE_WITH_C)

TREE_SITTER_BUILD_FILE_NO_C = """cc_library(
  name = "tree_sitter_{lang}",
  srcs = ["src/parser.c"],
  copts = ["-Iexternal/tree_sitter_{lang}/src"],
  hdrs = ["src/tree_sitter/parser.h"],
  linkstatic = True,
  visibility = ["//visibility:public"],
)
"""

def _tree_sitter_lang_impl_no_c(repository_ctx, lang):
  _tree_sitter_lang_impl_with_file(repository_ctx, lang, TREE_SITTER_BUILD_FILE_NO_C)

TREE_SITTER_BUILD_FILE_TS = """cc_library(
  name = "tree_sitter_typescript",
  srcs = ["typescript/src/parser.c", "typescript/src/scanner.c", "common/scanner.h"],
  copts = ["-Iexternal/tree_sitter_{lang}/typescript/src"],
  hdrs = ["typescript/src/tree_sitter/parser.h"],
  linkstatic = True,
  visibility = ["//visibility:public"],
)

cc_library(
  name = "tree_sitter_tsx",
  srcs = ["tsx/src/parser.c", "tsx/src/scanner.c", "common/scanner.h"],
  copts = ["-Iexternal/tree_sitter_{lang}/tsx/src"],
  hdrs = ["tsx/src/tree_sitter/parser.h"],
  linkstatic = True,
  visibility = ["//visibility:public"],
)
"""

def _tree_sitter_lang_impl_ts(repository_ctx, lang):
  _tree_sitter_lang_impl_with_file(repository_ctx, lang, TREE_SITTER_BUILD_FILE_TS)

def _tree_sitter_lang_impl_js(repository_ctx, lang):
  _tree_sitter_lang_impl_with_file(repository_ctx, lang, TREE_SITTER_BUILD_FILE_WITH_C)

# python
tree_sitter_python = repository_rule(
  implementation = lambda repository_ctx: _tree_sitter_lang_impl_with_cc(repository_ctx, lang = "python"),
)

# rust
tree_sitter_rust = repository_rule(
  implementation = lambda repository_ctx: _tree_sitter_lang_impl_with_c(repository_ctx, lang = "rust"),
)

# go
tree_sitter_go = repository_rule(
  implementation = lambda repository_ctx: _tree_sitter_lang_impl_no_c(repository_ctx, lang = "go"),
)

# typescript
tree_sitter_typescript = repository_rule(
  implementation = lambda repository_ctx: _tree_sitter_lang_impl_ts(repository_ctx, lang = "typescript"),
)

# javascript
tree_sitter_javascript = repository_rule(
  implementation = lambda repository_ctx: _tree_sitter_lang_impl_js(repository_ctx, lang = "javascript"),
)