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
