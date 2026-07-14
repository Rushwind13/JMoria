# Integration Plan: JMoria Agentic Framework

## Overview
This plan documents the integration of the agentic framework into the JMoria C++ roguelike project. The integration enables AI-assisted development by establishing host conventions, documenting gate capabilities, and creating role-specific overlays that context-aware agents.

## Work Items

### 1. Environment Probe
**Status**: ✅ COMPLETED

- **Interpreters & tooling**: Identified macOS 26.5.2, Apple clang 21.0.0, Make 3.81, Git 2.50.1, C++17
- **Build verification**: `make ascii` succeeds; executable 543K in size
- **Dependencies**: SDL2 (2.32.10), ncurses, OpenGL framework present and working
- **Test infrastructure**: Cucumber-CPP framework in place; googletest hardcoded path documented
- **Findings**: Build is healthy; no platform issues detected on macOS

### 2. Gate Mapping
**Status**: ✅ COMPLETED

- **G0 (Code style)**: clang-format documented; pre-commit hook template exists but not installed
  - Gap identified: CI does not enforce code style
  - Remediation: Manual review or install pre-commit hook
- **G1 (Build integrity)**: GitHub Actions `agentic-render-check.yml` working; verifies rendered agents ✓
- **G2 (Approval)**: Not configured (no branch protection rules in repository)
- **G3 (Deploy)**: Not applicable; no CD pipeline; merges to develop are information-only
- **Host gate set**: Only G1 is automated; G0 policy-documented, G2–G3 not configured

### 3. Conventions Map
**Status**: ✅ COMPLETED

- **Code style**: `.clang-format` exists; all developers must format before commit
- **Build**: Single Makefile with platform detection; three render modes; `uname` branching
- **Resources**: Monsters.txt, Items.txt in custom text format; parsed by FileParse
- **Testing**: Cucumber-CPP in test/; feature files + C++ step definitions
- **Documentation**: In doc/; Developer-Setup-Guide.md, coding-standards.md, architecture notes
- **Readability**: All conventions readable by fresh-clone agents; no gitignored convention files

### 4. Integration Profile
**Status**: ✅ COMPLETED

- **File**: `.agentic/runs/000-integration/integration-profile.md`
- **Content**:
  - Environment probe results (interpreters, versions, build health)
  - Gate mapping (G0–G3 assignment, host's actual gates)
  - Conventions map (where rules live, readability assessment)
  - Guardrail register (7 guardrails, each traced to probe finding or policy)
  - Decorrelation assessment (vendor reachability, P5 satisfaction)
  - Dispatch reality (interactive-only v1; no automation)
  - GI review gate (pending human approval)

### 5. Project Layer: Universal Overlays
**Status**: ✅ COMPLETED

- **File**: `.agentic/overlays/_all.md`
- **Content**: Universal guardrails, code style (G0), platform compatibility (G2), framework integrity (G1), resource data, testing, security, documentation, branches/releases

### 6. Project Layer: Role-Specific Overlays
**Status**: ✅ COMPLETED

All eight roles have context-specific overlays:

| Role | Overlay | Key Content |
|------|---------|-------------|
| Integrator | `integrator.md` | Host context (state machine, data-driven design, cross-platform); key guardrails; reference to integration profile |
| Analyst | `analyst.md` | Codebase overview (80 src files, parser utilities, state system); key architecture files to read; data format notes |
| Architect | `architect.md` | Design principles (state machine, data-driven, render abstraction, cross-platform); checklist for new features; known debt |
| Implementer | `implementer.md` | Build workflow (make targets, clang-format, test targets); common tasks (adding monsters/items/modes); guardrails; debugging tips |
| Reviewer | `reviewer.md` | PR checklist (architecture fit, code quality, platform compatibility, testing, documentation); red flags; review tools |
| Verifier | `verifier.md` | Test execution checklist (build, format, tests, manual play); platform verification; known test traps; acceptance template |
| Ops | `ops.md` | Release model (semantic versioning, develop branch, CI status); troubleshooting (build fails, test link errors, resource files); monitoring |
| Orchestrator | `orchestrator.md` | Status (not deployed v1); prerequisites for future deployment; example dispatch sequence; reference to integration profile |

### 7. Rendered Agent Files
**Status**: ✅ COMPLETED

- **Command**: `python3 .agentic/scripts/render-agents.py`
- **Output**: Regenerated all `.github/agents/*.agent.md` files
- **Verification**: `render-agents.py --check` passes; all agents up to date
- **Included**: Core role instructions + universal guardrails + role-specific overlays

### 8. Registry Binding Verification
**Status**: ✅ COMPLETED

- **File**: `.agentic/registry/models.yaml`
- **Status**: All roles pinned to Anthropic models (claude-fable-5, claude-sonnet-5, claude-haiku-4-5)
- **Action**: No changes needed; registry already configured
- **Note**: Alternates for OpenAI/Google listed; current binding is Anthropic-only

### 9. Validation & Framework Integrity
**Status**: ✅ COMPLETED

- **Build check**: `make ascii` still succeeds after overlay changes ✓
- **Framework check**: `render-agents.py --check` passes; no stale files ✓
- **No core edits**: All changes in overlays/ and rendered outputs; .agentic/contracts/, roles/, scripts/ untouched ✓
- **Git status**: Changes tracked; overlays and rendered files ready to commit

### 10. Intent Brief & Plan
**Status**: ✅ COMPLETED

- **Intent brief**: `.agentic/runs/000-integration/intent-brief.md` — objective, scope, phases, success criteria, notes for successors
- **This plan**: `.agentic/runs/000-integration/plan.md` — work items, status, deliverables

## Deliverables

### Integration Artifacts (in `.agentic/runs/000-integration/`)
- ✅ `integration-profile.md` — Complete profile with all sections
- ✅ `intent-brief.md` — Objective, scope, phases, success criteria
- ✅ `plan.md` — This document; work item tracking

### Project Layer (in `.agentic/overlays/`)
- ✅ `_all.md` — Universal guardrails (code style, platform, framework, resources, testing, security, docs, branches)
- ✅ `integrator.md` — Integrator context
- ✅ `analyst.md` — Analyst codebase overview
- ✅ `architect.md` — Architect design principles
- ✅ `implementer.md` — Implementer development workflow
- ✅ `reviewer.md` — Reviewer PR checklist
- ✅ `verifier.md` — Verifier test execution
- ✅ `ops.md` — Ops deployment & release
- ✅ `orchestrator.md` — Orchestrator future status

### Rendered Agents (in `.github/agents/`)
- ✅ `analyst.agent.md`
- ✅ `architect.agent.md`
- ✅ `implementer.agent.md`
- ✅ `reviewer.agent.md`
- ✅ `verifier.agent.md`
- ✅ `ops.agent.md`
- ✅ `integrator.agent.md`

## Key Decisions Made

1. **Gate Mapping**: Identified G1 as only automated gate (render-check); G0 (code style) and G2–G3 are policy-documented but not machine-enforced
2. **In-Repo Runs**: Runs directory at `.agentic/runs/` (in-repo); appropriate for open-source project with no sensitive state
3. **No Orchestrator Deployment**: Framework ready but orchestration v1 not deployed; future phase; all dispatch is human-initiated
4. **Overlays Over Edits**: All project policy in `overlays/` only; framework core copies remain read-only and generated
5. **Platform Compatibility**: All build scripts must support Darwin/Linux; Makefile uname branching is standard; changes must be tested on both

## Known Issues & Deferred Actions

| Issue | Status | Action |
|-------|--------|--------|
| G0 (code style) not automated | Gap | Pre-commit hook template exists in `doc/`; recommend manual installation or CI gate setup |
| Test dependencies not in CI | Gap | Documented in `doc/Developer-Setup-Guide.md`; manual setup required; flag for future CI enhancement |
| Hardcoded googletest path | Debt | Works on macOS Homebrew; Linux may differ; documented in `overlays/implementer.md` |
| Branch protection not configured | Policy | No machine-enforced approval gate; human review via PR required; acceptable for small/active team |

## Next Steps (For GI Review & Successor Agents)

1. **GI Review Gate**: Human reviewer approves integration profile; signs off on gateset, guardrails, dispatch reality
2. **Commit & Push**: Overlays, rendered agents, and run artifacts committed to develop branch
3. **Subsequent Runs**: Analyst, Architect, Implementer, Reviewer, Verifier can now be dispatched with full context
4. **Future Enhancement**: Deploy orchestrator v2 if automated dispatch sequences become valuable

## Success Criteria Checklist

- ✅ Integration profile is complete and reviewable (15 min for human GI)
- ✅ All seven guardrails trace to probe findings or host policies
- ✅ Overlays reflect JMoria's actual conventions and architecture
- ✅ Rendered agents incorporate project context; each role knows JMoria
- ✅ Framework files are read-only; overlays are writable
- ✅ Build still works (`make ascii` succeeds)
- ✅ All deliverables in place; ready for GI review

## Timeline

- **2026-07-14**: Integration probe completed; environment verified; all artifacts drafted
- **2026-07-14**: Overlays finalized; rendered agents regenerated
- **2026-07-14**: Integration run complete; awaiting GI review
- **[GI decision]**: Approved or requested changes
- **[Post-approval]**: First analyst/architect/implementer dispatches

---

**Integration Status**: Ready for GI Review  
**Framework Version**: 1.0.70 (Copilot CLI)  
**Host**: JMoria / Rushwind13/JMoria (GitHub)  
**Completed by**: Integrator (Copilot CLI)  
**Date**: 2026-07-14
