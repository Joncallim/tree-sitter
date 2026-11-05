// Exact linear range-selection reference from tree-sitter 0.25.10.
// This standalone component test includes the actual runtime implementation;
// it does not require an app, grammar, user document or native UI.
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/lib.c"

static void linear_reference_goto(Lexer *self, Length position) {
  if (position.bytes != self->current_position.bytes) {
    ts_lexer__invalidate_column_data(self);
  }

  self->current_position = position;

  // Move to the first valid position at or after the given position.
  bool found_included_range = false;
  for (unsigned i = 0; i < self->included_range_count; i++) {
    TSRange *included_range = &self->included_ranges[i];
    if (
      included_range->end_byte > self->current_position.bytes &&
      included_range->end_byte > included_range->start_byte
    ) {
      if (included_range->start_byte >= self->current_position.bytes) {
        self->current_position = (Length) {
          .bytes = included_range->start_byte,
          .extent = included_range->start_point,
        };
      }

      self->current_included_range_index = i;
      found_included_range = true;
      break;
    }
  }

  if (found_included_range) {
    // If the current position is outside of the current chunk of text,
    // then clear out the current chunk of text.
    if (self->chunk && (
      self->current_position.bytes < self->chunk_start ||
      self->current_position.bytes >= self->chunk_start + self->chunk_size
    )) {
      ts_lexer__clear_chunk(self);
    }

    self->lookahead_size = 0;
    self->data.lookahead = '\0';
  }

  // If the given position is beyond any of included ranges, move to the EOF
  // state - past the end of the included ranges.
  else {
    self->current_included_range_index = self->included_range_count;
    TSRange *last_included_range = &self->included_ranges[self->included_range_count - 1];
    self->current_position = (Length) {
      .bytes = last_included_range->end_byte,
      .extent = last_included_range->end_point,
    };
    ts_lexer__clear_chunk(self);
    self->lookahead_size = 1;
    self->data.lookahead = '\0';
  }
}

static uint32_t random_state = 0x13579bdf;
static uint32_t next_random(void) {
  random_state = random_state * 1664525u + 1013904223u;
  return random_state;
}
static TSPoint point_for(uint32_t byte) {
  return (TSPoint){.row = byte / 16, .column = byte % 16};
}
int main(void) {
  uint64_t checks = 0;
  static const char chunk[] = "0123456789abcdefghijklmnopqrstuv";
  for (uint32_t trial = 0; trial < 10000; trial++) {
    Lexer lexer;
    ts_lexer_init(&lexer);
    TSRange ranges[32];
    uint32_t count = trial % 33;
    uint32_t end = 0;
    for (uint32_t i = 0; i < count; i++) {
      uint32_t start = end + next_random() % 4;
      end = start + next_random() % 16;
      ranges[i] = (TSRange){.start_byte = start, .end_byte = end,
        .start_point = point_for(start), .end_point = point_for(end)};
    }
    assert(ts_lexer_set_included_ranges(&lexer, ranges, count));
    uint32_t limit = count ? end + 16 : 64;
    for (uint32_t byte = 0; byte <= limit; byte++) {
      lexer.current_position = (Length){.bytes = trial % 17, .extent = point_for(trial % 17)};
      lexer.chunk = chunk;
      lexer.chunk_start = trial % 7;
      lexer.chunk_size = sizeof(chunk) - 1;
      lexer.lookahead_size = 4;
      lexer.data.lookahead = 'x';
      lexer.column_data = (ColumnData){.value = 9, .valid = true};
      Lexer expected = lexer;
      Length position = {.bytes = byte, .extent = point_for(byte)};
      linear_reference_goto(&expected, position);
      ts_lexer_goto(&lexer, position);
      assert(lexer.current_position.bytes == expected.current_position.bytes);
      assert(lexer.current_position.extent.row == expected.current_position.extent.row);
      assert(lexer.current_position.extent.column == expected.current_position.extent.column);
      assert(lexer.current_included_range_index == expected.current_included_range_index);
      assert(lexer.chunk == expected.chunk && lexer.chunk_size == expected.chunk_size);
      assert(lexer.lookahead_size == expected.lookahead_size);
      assert(lexer.data.lookahead == expected.data.lookahead);
      assert(lexer.column_data.valid == expected.column_data.valid);
      assert(lexer.column_data.value == expected.column_data.value);
      checks++;
    }
    ts_lexer_delete(&lexer);
  }
  printf("lexer seek equivalence: %llu checks across 10000 sorted range sets\n", (unsigned long long)checks);
  return 0;
}
