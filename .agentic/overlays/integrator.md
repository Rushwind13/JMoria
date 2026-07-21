<!-- Project policy overlay. Non-comment content here is spliced into
     the integrator rendered agent(s) by render-agents.py. This is the ONLY writable
     policy surface: never edit core role copies in place. -->

## Integrator Role Specifics for JMoria

### Integration Success Criteria
A successful integration for JMoria means:
1. **Platform matrix tested:** Builds pass on macOS (primary), Linux (Docker OK), conceptual Raspberry Pi support validated (Makefile conditional)
2. **CI gate functional:** `agentic-render-check` runs on push/PR, validates rendered framework files
3. **Framework visibility:** `.agentic/runs/` visible in repository; agent run records part of project history
4. **No guardrail breaks:** Code submitted by any agent respects G1–G7 guardrails without human override
5. **Cold-start capability verified:** Fresh clone can immediately run `make ascii`, `make verify` without external setup beyond OS package install

### Probe Points (Findings Already Recorded)
- ✅ Environment: g++, GNU Make, Homebrew (macOS), platform detection via uname
- ✅ Gate: Single CI gate (render-check); no CD pipeline
- ✅ Conventions: Accessible in-repo (Copilot instructions, Developer's Guide, Makefile)
- ✅ Cross-platform: Makefile conditionals in place; Linux support unverified in CI
- ⚠️ Test infrastructure: Requires full build pre-step; sequential `make build` then `runtests.sh`

### Documentation Artifacts Created
- **Integration profile:** `.agentic/runs/000-integration/integration-profile.md` (this run's findings)
- **Project layer:** `overlays/_all.md` (global policy), role-specific overlays (agent guidelines)
- **Registry bindings:** `registry/models.yaml` (model assignments for each role)

### Known Risks & Mitigations
| Risk | Mitigation |
|------|-----------|
| Linux build not CI-tested; implementer changes might break it | Require manual Linux verification or Docker test before merge; document in overlay |
| Wizard Mode score-save constraint not enforced by compiler | Code review checklist; automated linting could detect `SaveScore()` without `IsWizardMode()` guard |
| Test executable wire protocol fragile (manual invocation breaks it) | Developer docs + in-code comments; ops runbook should highlight this |
| Resource file parsing custom format (not JSON/YAML) | Backward-compatible; agents should use existing FileParse utilities, not rewrite parsing |

### Integrator's Sign-Off Requirements
- [ ] All guardrails (G1–G7) trace to verified probe findings or cited host policy
- [ ] Role-specific overlays are non-empty and actionable (not just templates)
- [ ] Registry bindings reviewed for model tier appropriateness (frontier-reasoning for architect, balanced for implementer)
- [ ] No manual edits to `.agentic/roles/` or `.agentic/contracts/`; all project policy in `overlays/`
- [ ] GI human approver review scheduled
