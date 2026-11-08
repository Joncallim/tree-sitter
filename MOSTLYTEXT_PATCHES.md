# MostlyText runtime patches

Exact base: tree-sitter0.25.10 da6fe9beb4f7f67beb75914ca8e0d48ae48d6406.
Original MIT licence, public headers and ABI remain unchanged. This owner-
maintained fork is not an upstream submission or an upstream-endorsed fix.

The lexer previously scanned included ranges from index0 on each seek. Valid
ranges have nondecreasing end bytes. Lower-bound binary search finds the first
end after the original seek byte; the existing forward skip then preserves
empty-range, position, chunk, point and EOF behavior. No allocation or new public
interface is introduced. That range-lookup patch changes only lib/src/lexer.c in the runtime product.

lib/tests/lexer_seek_equivalence.c includes the actual runtime plus the original
linear implementation as an independent reference. It verifies1,624,448 cases
across10,000 sorted range sets, including empty/adjacent ranges, gaps, chunk and
column state and past-EOF positions. Original and patched versions pass; patched
AddressSanitizer/UndefinedBehaviorSanitizer passes. Compile standalone with:

    clang -O2 -std=c11 -D_DEFAULT_SOURCE -Ilib/include -Ilib/src \
      lib/tests/lexer_seek_equivalence.c -o /tmp/lexer-seek-equivalence
    /tmp/lexer-seek-equivalence

SwiftPM's existing sources:["src"] excludes this standalone test main from the
product. The documented cargo xtask test passes262 tests before/after. Broader
workspace tests pass317 unit cases but5 Rustdoc examples fail for missing
example variables/devgrammar; identical failures reproduce at the unmodified
base. No assertion or documentation example is disabled to turn this green.

A separate application-owned all-step corpus compares6,000 tree/query traces
across UTF-8/UTF-16LE/UTF-16BE. All old/new-runtime traces and diagnostics are
byte-identical, including282 original-runtime incremental/fresh discrepancies.
Both runs keep exit2; this is behavior-preservation proof, not6,000 fresh-equality
passes. Sanitized traces match without sanitizer diagnostics. Application
SwiftTreeSitter/MarkdownInline's explicit fresh-tree policy mitigates that
separate reused-tree discrepancy; it is not a claim about this lexer patch.

Synthetic many-range performance improves, but full editor latency, native
acceptance, exact release artifacts and the existing large-document fallback
remain separate application gates. Application adoption requires immutable
transitive pins, original licence/inventory checks and complete exact-head CI.

## Shared immutable subtree comparison

The comparison loop now avoids pushing a pair of identical heap child trees.
Both children must be non-inline and have the same pointer. Identical immutable
subtrees have identical descendants; distinct trees retain the original reverse
traversal, symbol/child-count ordering and equality behavior. Reference counts,
public headers, API/ABI, parser/scanner/edit state and original MIT text do not
change. Only lib/src/subtree.c changes in this additional runtime patch.

lib/tests/subtree_compare_identity.c includes the actual runtime and real JSON
parse trees. It checks324 ordered pairs, antisymmetry/transitivity, distinct
structurally equal trees, unequal descendants, shared copies and retained root
reference counts. Original and patched outputs are identical; AddressSanitizer
and UndefinedBehaviorSanitizer also produce that exact output without errors.
A disposable counter-only source copy establishes the work regression: the
original shared-document comparison visits36,866 pairs and fails the explicit
four-pair bound; the patched runtime visits2 and passes. No counters ship.

The actual current application inlineDense fixture and its root-derived inline
injection ranges reproduce150,921,270 comparison visits at262,151 bytes. The
patch reduces that to5,454 with identical complete tree/range/point/query hashes
for original, incremental and fresh parses. The documented262-test Rust/corpus
suite passes before and after. All6,000 valid-scalar behavior traces across
UTF-8/UTF-16LE/UTF-16BE remain byte-identical, including the same282 existing
raw incremental/fresh discrepancies and nonzero exit2. Sanitized traces match.
A simpler prose fixture makes zero comparisons in either version and establishes
no benefit for that input; it is retained rather than substituted for the
representative inline workload.

This is a component optimization, not proof of50ms editor latency, a larger
highlighting cutoff, the historical Debug assertion's cause, or signed/native
acceptance. Application adoption still requires current immutable transitive
pins, provenance and complete exact-head app/package/Debug/Release checks.
No upstream submission or source-offer/public release is claimed.

## Combined runtime compilation in SwiftPM

The Swift package now compiles the existing src/lib.c combined entry point plus
the separate conditional src/wasm/stdlib.c. This replaces the historical
sources:["src"] list and removes lib.c from exclusions. The twelve C files
included by lib.c plus stdlib.c are exactly the same thirteen implementation
inputs as before. Original C11, feature/platform defines, headers, source bytes
and licence texts remain unchanged. The standalone controls remain outside the
product source list. No parser, grammar, query or runtime algorithm changes in
this build-only step, and the rejected capture-sorting experiment is excluded.

A normal Foundation console using the exact actual wrapper and Markdown
grammar/query sources compares the original and combined package builds. All
23,256 ordered highlight results, named/global metadata, current-source ranges,
aliases and independent concurrent cursor controls agree. All260 defined external
C symbol names agree, and recorded actual compiler/linker inputs retain the same
C defines/standard/optimization flags. Two paired five-measurement comparisons
show approximately9.5% and11% lower local total medians. These component timings
do not establish a whole-app budget or native acceptance.

The earlier complete262-case Rust/corpus and C sanitizer/ownership/trace evidence
applies to the unchanged C sources and already uses the combined entry point;
it does not independently qualify this Swift package-build change. Application
adoption still requires fresh exact locked resolution, full package/app tests
and the unchanged Debug/Release native/integration/performance gates. No source
offer, upstream submission, signing or public release is performed.

## Included-range reuse and UTF-16 decoder correctness

This is a narrow source update over the exact combined-runtime revision
`3b1fb00f8c4f8907073f9b7342a807a0231867fa`. The proposed product delta
is limited to `lib/src/parser.c` and `lib/src/unicode.h`; the original MIT
licence, public C headers, Swift manifest, thirteen compiled C inputs and
existing allocation/ownership policy remain unchanged.

An included-range change can cross bytes a cached token examined just beyond
its own span. The reviewed upstream
[lookahead fix](https://github.com/tree-sitter/tree-sitter/pull/5641) extends the
existing reuse comparison through that lookahead for non-EOF tokens. The
existing EOF handling is retained. In the exact pinned Markdown-inline grammar,
eight fixed UTF-8 and UTF-16LE range-change cases had sixteen incremental/fresh
tree mismatches before this guard and none after it. The application currently
parses Markdown inline afresh, so this is a C component correctness result,
not proof of an editor-visible Markdown bug or improved typing latency.

The same pinned runtime passes UTF-16 decoder byte lengths to macros that count
16-bit units. The reviewed upstream
[bounds fix](https://github.com/tree-sitter/tree-sitter/pull/5549) rejects chunks
shorter than one unit and converts the supplied byte length to units. Both
original decoders read past a true two-byte allocation when the advertised
chunk ends with a lead surrogate; AddressSanitizer reports a two-byte
heap-buffer-overflow. A separate reviewed upstream
[byte-order fix](https://github.com/tree-sitter/tree-sitter/pull/5912) converts
the trailing surrogate in its declared endian order before testing and
combining it. The original twelve-case decoder matrix has six failed cases;
the bounds-only source has one remaining valid big-endian pair failure; the
two exact upstream corrections together pass all twelve. The original,
bounds-only and combined first results remain separately retained.

Before publication, require the same unchanged C fixtures against the exact
committed source, the eight-case included-range corpus, all 324 ordered
subtree comparison/ownership checks, strict compile and AddressSanitizer plus
UndefinedBehaviorSanitizer. Compare all product source bytes, exported symbols
and licence to the pinned parent. The standalone C tests live in `lib/tests`,
outside the Swift package product source list. They receive fixed generated
input only. No application, document, GUI or JavaScript execution is involved.

Application adoption must update the dependency graph and compliance inventory
together, resolve the actual lock through the authorized hosted runner, then
qualify package, app, native editing and original regression paths on one exact
candidate. These C results do not close the original Debug abort, 50 ms budget,
attribution or release gates.

The fork's `lib/tests/utf16_decoder_boundaries.c` is the unchanged twelve-case
driver from the retained original/bounds/composite comparison. It runs directly
against this fork's `unicode.h`:

    clang -std=c11 -D_DEFAULT_SOURCE -O2 -Wall -Wextra -Werror \
      -Ilib/src -Ilib/include lib/tests/utf16_decoder_boundaries.c \
      -o /tmp/utf16-decoder-boundaries
    /tmp/utf16-decoder-boundaries

The actual fork source passes all twelve normal and instrumented cases; the
two true two-byte heap probes pass with AddressSanitizer and
UndefinedBehaviorSanitizer. The same fork source yields the earlier exact
eight-case Markdown-inline output and all 324 ordered subtree results, without
sanitizer findings. The 268 defined external C symbols agree with the original
runtime. The proof retains actual commands, binary hashes, linkage and full
stdout/stderr under
`/Volumes/Fast SSD/CORTANA-Builds/parser-fork-source-20261108/local-first`.
Those local component results do not replace exact hosted source/lock/native
qualification.
