# GI Reviewer Quick Start

**You are the GI (Gateway Integration) human approver.** Your job is to verify that this integration is sound and ready for agents to operate in JMoria. This should take ~15 minutes.

---

## Files to Review (in order)

### 1. **SUMMARY.md** (5 min) ← START HERE
Executive summary of integration findings, deliverables, and known gaps. Read first to get oriented.

### 2. **integration-profile.md** (10 min) ← CORE DOCUMENT
The complete integration profile per contract specification:
- **Environment probe** — macOS, clang, Make, dependencies verified
- **Gate mapping** — G0–G3 assessment; only G1 is automated
- **Conventions map** — all live in-repo; readable by cold-start agents
- **Guardrail register** — seven guardrails, each traced to findings
- **Decorrelation assessment** — vendor and plan persistence status
- **Dispatch reality** — human-only v1; orchestrator not deployed

**Key sections to verify**:
- Are environment findings accurate?
- Is gate mapping correct for your organization?
- Do the seven guardrails align with JMoria's culture?
- Any concerns about known gaps (G0 not automated, test CI not setup)?

### 3. **Overlays/** (5 min) ← SPOT-CHECK
Sample a few role overlays to ensure they reflect JMoria's actual practices:
- `overlays/_all.md` — universal guardrails (read first)
- `overlays/implementer.md` — check if build workflow matches reality
- `overlays/architect.md` — check if design principles match JMoria's philosophy

**Quick spot-check questions**:
- Does the implementer overlay correctly describe the build workflow?
- Does the architect overlay reflect JMoria's state machine architecture?
- Are the code style rules (clang-format) accurate?

### 4. **Rendered Agents** (2 min) ← VERIFICATION
Check that rendered agents were regenerated and are current:
```bash
python3 .agentic/scripts/render-agents.py --check
# Expected output: "all rendered agents up to date"
```

---

## Integration Snapshot

| Aspect | Status | Notes |
|--------|--------|-------|
| **Build** | ✅ | `make ascii` succeeds; executable created |
| **Framework** | ✅ | Render-check CI passes; no stale files |
| **Probe** | ✅ | macOS verified; conventions mapped |
| **Overlays** | ✅ | 9 files; 550 lines; role-specific context |
| **Guardrails** | ✅ | 7 rules; each traced to findings |
| **G0 (code style)** | ⚠️ | Documented; template exists; not yet enforced |
| **G1 (render-check)** | ✅ | GitHub Actions workflow working |
| **G2–G3** | ❌ | Not configured (acceptable for active team) |

---

## Questions for Approval

Answer yes to all to approve:

1. **Environment is accurate?** (macOS, clang-format, Makefile, dependencies)
   - Review in: `integration-profile.md` → "Environment probe"

2. **Gate mapping is correct?** (G1 automated, G0–G3 as noted)
   - Review in: `integration-profile.md` → "Gate mapping"

3. **Conventions are readable?** (All in-repo; no gitignored rules)
   - Review in: `integration-profile.md` → "Conventions map"

4. **Guardrails are appropriate?** (Seven rules; each traced)
   - Review in: `integration-profile.md` → "Guardrail register"

5. **Role overlays reflect JMoria?** (Architecture, build, code style)
   - Spot-check: `overlays/_all.md`, `overlays/implementer.md`, `overlays/architect.md`

6. **No core framework files edited?** (Only overlays + renders)
   - Verify: `git status` shows only `.agentic/overlays/` and `.github/agents/` modified

7. **Build still works?** (Executable created; no corruption)
   - Verify: `./jmoria` executable exists (543K)

8. **Known gaps acceptable?** (G0 not automated, test CI missing, hardcoded paths)
   - Review in: `integration-profile.md` → "Guardrail register" and `SUMMARY.md`

---

## Approval Decision

**If YES to all 8 questions above:**
```
approved: true
by: Jimbo
at: 7/14/2026 2:00pm
notes: initial configuration, no notes
```

Then update `.agentic/runs/000-integration/integration-profile.md` → "Gate GI record" section.

**If NO or UNCERTAIN:**
- Specify which questions need clarification or remediation
- Integrator will address and re-submit

---

## What Happens After Approval

1. Overlays + rendered agents are committed to `develop` branch
2. Analyst can be dispatched for codebase scan (optional)
3. Architect can review design for next phase
4. Implementer can work on new features with full context
5. All roles understand JMoria's architecture, build, conventions

---

## Reference Documents

- `integration-profile.md` — Primary artifact (12K, 230+ lines)
- `plan.md` — Work tracking
- `intent-brief.md` — Objective & scope
- `state.yaml` — Machine-readable state
- `SUMMARY.md` — This-is-the-executive-summary
- All in: `.agentic/runs/000-integration/`

---

## Quick Validation Commands (Copy-Paste)

```bash
# Verify framework is healthy
python3 .agentic/scripts/render-agents.py --check

# Verify build works
make clean && make ascii

# Verify executable created
ls -lh jmoria

# List all integration artifacts
ls -lh .agentic/runs/000-integration/

# List all overlays
ls -lh .agentic/overlays/

# Check git status (should show only framework files modified)
git status --short
```

---

**Estimated review time**: 15 minutes  
**Decision point**: GI record in `integration-profile.md`  
**Framework version**: 1.0.70  
**Host**: JMoria (C++ Roguelike)
