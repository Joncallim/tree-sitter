# MostlyText range-lookup patch

Exact base: tree-sitter0.25.10 da6fe9beb4f7f67beb75914ca8e0d48ae48d6406.
Original MIT licence, public headers and ABI remain unchanged. This owner-
maintained fork is not an upstream submission or an upstream-endorsed fix.

The lexer previously scanned included ranges from index0 on each seek. Valid
ranges have nondecreasing end bytes. Lower-bound binary search finds the first
end after the original seek byte; the existing forward skip then preserves
empty-range, position, chunk, point and EOF behavior. No allocation or new public
interface is introduced. Only lib/src/lexer.c changes in the runtime product.

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
