<!-- Project policy overlay. Non-comment content here is spliced into
     the orchestrator rendered agent(s) by render-agents.py. This is the ONLY writable
     policy surface: never edit core role copies in place. -->

## Orchestrator: JMoria Autonomous Dispatch (v1 Setup)

### Current Status
- **Not deployed**: Orchestrator workflow not integrated into CI yet
- **Runs are human-initiated**: Integrator, Analyst, Architect, Implementer dispatches via manual calls
- **Framework ready**: `.agentic/` structure supports orchestration; no execution scheduled

### When to Deploy Orchestrator (Future Phase)

An orchestrator would be useful for JMoria if:
1. **Regular analysis runs**: E.g., weekly codebase health checks, technical debt scanning
2. **Automated issue triage**: Categorize GitHub issues and suggest analysis/architecture tasks
3. **Pre-release CI**: Automatically run full analysis suite before cutting releases
4. **Continuous improvement**: Periodic architect reviews of new code patterns

### Orchestrator Constraints in This Host

**What works**:
- ✓ Framework files in place; run directory support exists (in-repo `.agentic/runs/`)
- ✓ All roles have overlays with JMoria context
- ✓ Budget ledger in `registry/models.yaml` (pre-configured)

**What's missing** (would need to implement):
- ❌ No orchestrator GitHub Actions workflow (would schedule dispatch)
- ❌ No dispatch manifest (defining which runs to execute and in what order)
- ❌ No webhook handlers (GitHub issues → orchestrator triggers)
- ❌ No automated gate entry (orchestrator must stay behind human gate; v1 rule)

### Prerequisites for Orchestrator Deployment

1. **Define dispatch sequence**: Map out roles and their task dependencies
   - Example: Analyst → Architect → Implementer (serial); Reviewer ↔ Implementer (parallel)
2. **Create dispatch manifest**: `dispatch.yaml` defining run sequence, budget, approval gates
3. **Add GitHub Actions workflow**: `orchestrate.yml` (scheduled or webhook-triggered)
4. **Budget capacity**: Verify model costs in `registry/models.yaml` fit project budget
5. **Human gate**: Orchestrator produces recommendations; human decides dispatch or tweaks

### Example Future Orchestrator Task

```yaml
# Hypothetical: Weekly codebase health check
name: weekly-health-check
schedule: "0 9 * * MON"  # Monday 9 AM
sequence:
  - analyst: scan for code smells, test coverage gaps
  - architect: review scan results, flag design concerns
  - verifier: assess health report, recommend actions
- approval: human review before dispatch
```

### Integration Profile Reference
- `.agentic/runs/000-integration/integration-profile.md`
- **Dispatch reality section**: Documents that v1 is interactive-only; orchestrator not yet deployed
- Use to understand current automation readiness

### Not Required for This Integration

The integrator phase is **complete without orchestrator deployment**. Orchestrator is a future enhancement:
- Current: Analyst, Architect, Implementer roles are available; dispatch them manually via Copilot CLI
- Future: Automate dispatch sequences via orchestrator (out of scope)
- Stay on human gate: All dispatch entry points remain human-initiated (v1 framework rule)
