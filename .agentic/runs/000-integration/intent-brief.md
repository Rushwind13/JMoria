# Intent Brief: JMoria Integration

## Objective
Integrate the agentic framework into JMoria (C++ roguelike) to enable AI-assisted development workflows. Establish host conventions, gate mapping, and role overlays so that subsequent agents (analyst, architect, implementer, reviewer, verifier) can operate autonomously and correctly within JMoria's architecture and build environment.

## Scope
- **In scope**: Environment probe, gate mapping, conventions documentation, project overlays, framework validation
- **Out of scope**: Code changes, feature development, test setup (test dependencies are documented; not installed)

## Host Context
- **JMoria**: From-scratch C++ roguelike (homage to IMoria)
- **Architecture**: State machine (game modes as state classes), data-driven monsters/items (parsed from .txt files)
- **Platforms**: macOS (primary), Linux/Ubuntu, Raspberry Pi OS
- **Build**: Three modes — ASCII (ncurses), OpenGL (SDL2), or both; single Makefile with platform detection
- **CI**: Single GitHub Actions workflow (render-check on push/PR); no CD or branch protection configured

## Integration Phases

### Phase 1: Probe & Profile (COMPLETED)
- ✓ Environment probed: macOS 26.5.2, Apple clang 21.0.0, Make 3.81, C++17
- ✓ Build verified: `make ascii` succeeds; executable created
- ✓ Conventions mapped: clang-format, Makefile, resources, tests, documentation
- ✓ Gates identified: G1 (render-check) working; G0 (code style) documented but not automated
- ✓ Integration profile drafted: `.agentic/runs/000-integration/integration-profile.md`

### Phase 2: Overlays & Registry (COMPLETED)
- ✓ Project overlays created: `overlays/_all.md` (universal guardrails)
- ✓ Role overlays created: integrator, analyst, architect, implementer, reviewer, verifier, ops, orchestrator
- ✓ Each overlay tailored to role's interaction with JMoria (architecture patterns, build workflow, review criteria, etc.)
- ✓ Rendered agents regenerated: `.github/agents/*.agent.md` now reflect project context
- ✓ Registry verified: Model bindings in `registry/models.yaml` (Anthropic models for all roles)

### Phase 3: Validation (IN PROGRESS)
- [ ] CI render-check passes (framework files are consistent)
- [ ] Overlays committed alongside rendered agents
- [ ] Integration profile reviewed by GI human (gate entry)
- [ ] Integration run artifacts signed off

## Success Criteria
1. ✓ Integration profile completeness: Environment, gates, conventions, guardrails, dispatch reality documented
2. ✓ Overlays reflect JMoria's actual practices: State machine, data-driven design, cross-platform, clang-format
3. ✓ Rendered agents incorporate project context: Each role knows JMoria's architecture, build process, review criteria
4. ✓ Framework integrity verified: render-agents.py --check passes; no stale files
5. ✓ No core .agentic/ files edited: All changes in overlays/ and rendered outputs only
6. ✓ Build still works: `make ascii` succeeds after overlay changes (no file corruption)

## Exit Criteria (GI Review Gate)
- Integration profile signed off by human GI reviewer
- All overlays reflect host's actual policies and conventions
- No framework invariants broken
- Subsequent runs can be dispatched with confidence that agents understand JMoria

## Known Unknowns / Deferred
- **Test infrastructure**: Cucumber-CPP ready but dependencies not installed in CI; documented for manual setup
- **Code style gate (G0)**: Documented but not machine-enforced; pre-commit hook template exists but not yet installed
- **Orchestrator deployment**: Framework supports it; not needed for v1 integration; flagged for future phase
- **Hardcoded test paths**: googletest path in Makefile; works on current machine; Linux may differ

## Notes for Successor Agents
All information needed to work on JMoria is now in the integration profile and overlays:
- **Analyst**: Start with `integration-profile.md` and `overlays/analyst.md`
- **Architect**: Review `overlays/architect.md` for design principles and architectural patterns
- **Implementer**: Follow `overlays/implementer.md` build & development workflow
- **Reviewer**: Use `overlays/reviewer.md` for PR review checklist
- **Verifier**: See `overlays/verifier.md` for acceptance testing criteria

No agent needs to re-probe the environment; context is persistent in this run and subsequent runs.
