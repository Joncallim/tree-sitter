// Standalone component control over real parsed trees and the actual runtime.
// Compile with lib/src on the include path and the fixture JSON parser.
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib.c"

const TSLanguage *tree_sitter_json(void);

static int sign_of(int value) { return (value > 0) - (value < 0); }

int main(int argc, char **argv) {
  bool require_bound = argc == 2 && strcmp(argv[1], "--require-bounded-self-comparison") == 0;
  assert(argc == 1 || require_bound);
  static const char *sources[] = {
    "", "null", "true", "false", "1", "2", "\"café 雪 😀\"",
    "[]", "{}", "[1]", "[2]", "[\"a\"]", "{\"a\":[1,true,null]}",
    "{\"a\":[1,true,null]}", "{\"a\":[1,true,{}]}", "[1,\r\n2,\r3,\n4]",
    "{\"broken\":[1,", "[[[[[[]]]]]]",
  };
  enum { COUNT = sizeof(sources) / sizeof(sources[0]) };
  TSParser *parser = ts_parser_new();
  assert(ts_parser_set_language(parser, tree_sitter_json()));
  TSTree *trees[COUNT];
  SubtreePool pool = ts_subtree_pool_new(0);
  int ordering[COUNT][COUNT];
  for (unsigned i = 0; i < COUNT; i++) {
    trees[i] = ts_parser_parse_string(parser, NULL, sources[i], (uint32_t)strlen(sources[i]));
    assert(trees[i]);
  }
  for (unsigned i = 0; i < COUNT; i++) {
    uint32_t references = trees[i]->root.data.is_inline ? 0 : trees[i]->root.ptr->ref_count;
    for (unsigned j = 0; j < COUNT; j++) {
      ordering[i][j] = sign_of(ts_subtree_compare(trees[i]->root, trees[j]->root, &pool));
      assert(pool.tree_stack.size == 0);
      printf("compare %u %u %d\n", i, j, ordering[i][j]);
    }
    assert(trees[i]->root.data.is_inline || trees[i]->root.ptr->ref_count == references);
    assert(ordering[i][i] == 0);
  }
  // Equal structure from distinct parse allocations still compares equal.
  assert(trees[12]->root.ptr != trees[13]->root.ptr);
  assert(ordering[12][13] == 0 && ordering[12][14] != 0);
  for (unsigned i = 0; i < COUNT; i++) {
    for (unsigned j = 0; j < COUNT; j++) {
      assert(ordering[i][j] == -ordering[j][i]);
      for (unsigned k = 0; k < COUNT; k++) {
        if (ordering[i][j] <= 0 && ordering[j][k] <= 0) assert(ordering[i][k] <= 0);
      }
    }
  }

  const char *element = "{\"key\":[1,true,null,\"雪😀\"]},";
  size_t element_size = strlen(element), size = element_size * 1024 + 2;
  char *large = malloc(size + 1);
  assert(large);
  large[0] = '[';
  for (unsigned i = 0; i < 1024; i++) memcpy(large + 1 + i * element_size, element, element_size);
  large[size - 2] = ']';
  large[size - 1] = '\0';
  TSTree *large_tree = ts_parser_parse_string(parser, NULL, large, (uint32_t)strlen(large));
  assert(large_tree && !ts_node_has_error(ts_tree_root_node(large_tree)));
  TSTree *copy = ts_tree_copy(large_tree);
  assert(copy && copy->root.ptr == large_tree->root.ptr);
#ifdef OBSERVE_COMPARISON_WORK
  comparison_visit_count = 0;
#endif
  assert(ts_subtree_compare(large_tree->root, copy->root, &pool) == 0);
  assert(pool.tree_stack.size == 0);
#ifdef OBSERVE_COMPARISON_WORK
  fprintf(stderr, "SHARED_SUBTREE_WORK nodes=%llu\n", (unsigned long long)comparison_visit_count);
  if (require_bound && comparison_visit_count > 4) {
    fprintf(stderr, "Identical shared document descendants were traversed again\n");
    return 2;
  }
#else
  if (require_bound) {
    fprintf(stderr, "A disposable instrumented source copy is required for the work bound\n");
    return 64;
  }
#endif
  ts_tree_delete(copy);
  ts_tree_delete(large_tree);
  free(large);
  for (unsigned i = 0; i < COUNT; i++) ts_tree_delete(trees[i]);
  ts_subtree_pool_delete(&pool);
  ts_parser_delete(parser);
  puts("Actual tree comparison ordering, distinct equality, shared identity and reference ownership pass");
  return 0;
}
