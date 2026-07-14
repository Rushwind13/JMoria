<!-- Project policy overlay. Non-comment content here is spliced into
     the integrator rendered agent(s) by render-agents.py. This is the ONLY writable
     policy surface: never edit core role copies in place. -->

## Integrator: JMoria Host Context

You are integrating the agentic framework into **JMoria**, a from-scratch C++ roguelike game engine with:
- **State machine architecture**: Game modes as separate state classes; states transition via `CGame::SetState()`
- **Data-driven monsters/items**: Parsed from `Resources/Monsters.txt` and `Resources/Items.txt` (custom format)
- **Cross-platform build**: Darwin (macOS), Linux, Raspberry Pi OS via single Makefile with `uname` branching
- **Three render modes**: ASCII (ncurses), OpenGL (SDL2), or both at runtime

### Host's Current Integration Status
- ✓ Framework installed (`.agentic/`, `framework-lock.json`)
- ✓ CI workflow present (`agentic-render-check.yml`)
- ⚠️ Code style gate (G0) documented but not machine-enforced; pre-commit hook template exists
- ⚠️ Test infrastructure ready but test dependencies not yet installed in CI

### Key Guardrails for Integrator
1. **Do not edit core copies**: `.agentic/contracts/`, `.agentic/roles/`, `.agentic/scripts/` are read-only
2. **Overlays are writable**: Update `overlays/_all.md` and per-role overlays as needed for project policy
3. **After overlay changes**: Run `.agentic/scripts/render-agents.py` to regenerate `.github/agents/`; commit both
4. **Platform testing**: If you modify Makefile or build scripts, verify on macOS and at least document assumptions for Linux/Pi

### Integration Profile Location
- `integration-profile.md` in this run directory (`.agentic/runs/000-integration/`)
- Contains: environment probe, gate mapping, conventions, guardrails, dispatch reality
- Use as reference for subsequent runs and for human GI review
