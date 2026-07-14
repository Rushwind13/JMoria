# Review Report: <task id>

<!-- Contract: produced by Reviewer; consumed by Implementer and gate G2.
     All sections required. Findings ranked most-severe first.
     BUDGET: one line + failure scenario per finding — no narrative. Reference
     the spec and diff (requirement numbers, file:line); never re-quote them. -->

**Verdict:** approve | request-changes | escalate
**Round:** <n of 3>
**Diff reviewed:** <branch/commit>

## Findings

### F1 — <severity: blocking | major | minor> — <one-line defect>
- **Where:** `path/to/file.py:123`
- **Failure scenario:** <concrete inputs/state → wrong output or crash.
  If you can't construct one, mark the finding PLAUSIBLE.>
- **Requirement:** <spec/plan reference this violates, if applicable>

## Coverage
<!-- What you checked and found clean — the G2 human relies on this, not just
     the findings. E.g.: "requirement coverage R1-R3 ✓; error paths in X ✓;
     concurrency not assessed (no concurrent access in scope)". -->

## Boundary check
<!-- Did the diff stay inside the task's declared file_contact_surface? -->
