<!-- Project policy overlay. Non-comment content here is spliced into
     the orchestrator rendered agent(s) by render-agents.py. This is the ONLY writable
     policy surface: never edit core role copies in place. -->

## Orchestrator Role Specifics for JMoria

### Dispatch Model

**Dispatch mode:** Interactive operator (no autonomous scheduling)
- Agents invoked on-demand by user (GitHub Copilot CLI or direct run command)
- No self-dispatching daemon or job queue
- No scheduled sweeps (historian role configured but not activated)

### Integration Profile Reference
See `.agentic/runs/000-integration/integration-profile.md` for:
- Environment probes (g++, Homebrew, platform detection)
- Gate mapping (single CI gate: agentic-render-check)
- Conventions map (all on-disk, agent-readable)
- Guardrails (G1–G7, traced to probe findings)

### Budget Allocation

**Per-dispatch estimates (from registry/models.yaml):**
- Orchestrator: $0.50 (escalation summary, triage)
- Analyst: $2 (scope & feasibility)
- Architect: $5 (design review)
- Implementer: $8 (code changes)
- Reviewer: $4 (code review)
- Verifier: $6 (testing & validation)
- Ops: $2 (release/deployment)

**Note:** These are *estimates*; actual usage depends on task complexity. Monitor spend and escalate if a dispatch exceeds its estimate.

### Guardrail Enforcement Points

**G1 – Platform Compatibility:** 
- Escalate if implementer can't verify Linux build
- Request Docker-based Linux testing or reject Makefile changes

**G2 – Code Formatting:**
- Verifier checks clang-format compliance
- Reviewer enforces pre-commit hook setup

**G3 – Build Modes:**
- Verifier tests all three render modes (`make ascii`, `make opengl`, `make`)

**G4 – Data-Driven Design:**
- Reviewer rejects hardcoded game balance numbers in .cpp
- Enforce move to Resources/Monsters.txt or Resources/Items.txt

**G5 – Cross-Platform Includes:**
- Reviewer flags absolute paths in Makefile or code
- Enforce use of $(LOCAL_INCLUDE_PATHS), $(LOCAL_LIB_PATHS)

**G6 – Wizard Mode Scope:**
- Reviewer checks IsWizardMode() guard before SaveScore()

**G7 – Test Protocol:**
- Verifier ensures AllSteps runs only via runtests.sh, not manually
- Document this in test runbook

### Role Sequencing (Typical Dispatch)

**Feature request (high complexity):**
1. **Analyst** — Scope, feasibility, acceptance criteria
2. **Architect** — Design review, patterns, dependencies
3. **Implementer** — Code changes, testing
4. **Reviewer** — Code review, compliance check
5. **Verifier** — Build verification, platform testing
6. **Ops** — Release planning (if approved for release)

**Bug fix (low complexity):**
1. **Analyst** — Root cause analysis (optional, if complex)
2. **Implementer** — Code fix
3. **Reviewer** — Code review
4. **Verifier** — Regression testing

**Build/infrastructure:**
1. **Ops** — Release planning, dependency updates
2. **Verifier** — Build verification on all platforms
3. **Reviewer** — Makefile compliance

### Escalation Criteria

**Escalate to human (GI gatekeeper) if:**
- [ ] Guardrail G1 (platform compatibility) can't be verified without human decision
- [ ] Design decision conflicts with existing architecture (architect can't resolve)
- [ ] Resource estimate exceeds dispatch budget by >20%
- [ ] Security or data-sensitive issue discovered
- [ ] Multiple agent retries without resolution

**Example escalation message:**
> "Implementer cannot test Raspberry Pi build without hardware. Verifier has Docker Linux test passing; recommend accepting Makefile changes with caveat that Raspberry Pi testing deferred to next hardware availability."

### Dispatch Checklist

Before starting a dispatch:
- [ ] User intent is clear (feature, bug fix, refactor)
- [ ] Task fits within single dispatch (not multi-sprint)
- [ ] Integration profile is current (guards are up-to-date)
- [ ] Registry model bindings are approved (no vendor conflicts)
- [ ] Budget sufficient for estimated complexity

Before accepting dispatch result:
- [ ] All guardrails satisfied (G1–G7 checked)
- [ ] Role sequence complete (analyst → architect → implementer → reviewer → verifier)
- [ ] Tests pass (BDD scenarios green, no regressions)
- [ ] Platform verification done or deferred with rationale
- [ ] Handoff notes clear for next phase

### Known Constraints

- **No multi-phase work:** Each dispatch is bounded (typically 1–2 hours per role)
- **No persistent state:** Each role starts fresh; context passed via git branch + notes
- **No autonomous decisions:** Guardrail exceptions require human approval (GI gatekeeper)
- **Platform testing limited:** macOS primary; Linux via Docker; Raspberry Pi conceptual
