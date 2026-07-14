# Verification Report: <run or task id>

<!-- Contract: produced by Verifier; consumed by gate G2.
     Every in-scope acceptance criterion gets a row and evidence.
     Evidence = the command you ran and the output you observed.
     BUDGET: paste FAILING output in full; for passing checks the command plus
     its concluding line/exit code suffices. Never paste entire suites or
     restate the spec — reference criteria by number. -->

**Change verified:** <branch/commit>
**Environment:** <where this ran: local, CI, staging + versions that matter>

## Results

| Criterion | Verdict | Evidence |
|-----------|---------|----------|
| AC1.1 | verified / failed / unverifiable | see E1 |

### E1 — AC1.1
```
$ <command>
<observed output>
```
<!-- One evidence block per criterion. Failed runs are results too — paste them. -->

## Beyond the happy path
<!-- What you probed that the criteria didn't ask for (malformed input, empty
     states, boundaries, restarts) and what happened. -->

## Gaps
<!-- Criteria you could not verify and why; tests you added; coverage still missing. -->
