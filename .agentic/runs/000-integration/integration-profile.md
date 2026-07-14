# Integration Profile: JMoria

## Environment probe

### Interpreter and Build Tooling
- **OS**: macOS 26.5.2 (verified; Darwin kernel)
- **Compiler**: Apple clang version 21.0.0; g++ wrapper (targets Apple clang)
- **Make**: GNU Make 3.81 (available; functional)
- **Language**: C++17 standard
- **Git**: 2.50.1
- **clang-format**: 21.1.8 (Homebrew), available and working

### Build System
JMoria uses a cross-platform Makefile with three render modes:
- `make ascii` — ncurses terminal renderer; works on Darwin/Linux
- `make opengl` — SDL2+OpenGL; requires framework on macOS
- `make` (default) — both renderers, runtime selection via `--renderer` flag

**macOS-specific configuration** (verified working):
- Homebrew paths: `/usr/local/include`, `/opt/homebrew/include`
- SDL2 (2.32.10 via sdl2-config), SDL2_image, ncurses, OpenGL framework
- GoogleTest 1.17.0 hardcoded path: `/opt/homebrew/Cellar/googletest/1.17.0/`
- Build test run: `make ascii` succeeded; executable created at `./jmoria` (543K)

### Test Infrastructure
- **Test Framework**: Cucumber-CPP with GoogleTest
- **Test runner**: `test/runtests.sh` (requires Gemfile.lock, cucumber 7.1.0)
- **Test executable**: `test/bin/AllSteps` (wire protocol)
- **Feature files**: `test/features/*.feature`
- **Step definitions**: `test/features/step_definitions/` (C++ steps)
- **Test requirements** (not yet installed in this checkout): 
  - ruby, rubygems; `sudo gem install cucumber -v 7.1.0` 
  - `googletest`, `cucumber-cpp` via Homebrew or apt
- **Note**: Cucumber-cpp requires cucumber 7.1.0 (wire protocol); 8.0+ removed it

### Code Style & Hooks
- **clang-format config**: `.clang-format` exists, version 21.1.8 available
- **Pre-commit hook**: Template documented in `doc/Developer-Setup-Guide.md` (not yet installed)
  - Expected location: `.git/hooks/pre-commit` (currently absent)
  - Intended: run `git clang-format --diff --staged` and reject if format errors detected
  - Policy: All commits must pass `clang-format` via staged changes

### Package Management
- No package.json, requirements.txt, or equivalent — pure C++ with manual Makefile build
- Dependencies documented in `doc/Developer-Setup-Guide.md`; macOS via Homebrew, Linux via apt

### Conventions Readability
All conventions live in files readable by cold-start agents in a fresh clone:
- `.clang-format` — Copilot CLI can read directly
- `doc/Developer-Setup-Guide.md` — Build prerequisites and hook setup (prose)
- `doc/coding-standards.md` — Code style guidelines
- `Makefile` — Platform detection and build configuration (executable documentation)
- `.gitignore` — Exclusions documented and readable
- **Finding**: Pre-commit hook is documented but not installed; remediation: CI workflow or documentation note can enforce or guide setup

---

## Gate mapping

### GitHub Actions CI

**Current workflow**: `.github/workflows/agentic-render-check.yml`
- **Trigger**: `on: [push, pull_request]`
- **Job**: `render-check` on `ubuntu-latest`
- **Action**: `python3 .agentic/scripts/render-agents.py --check`
- **Purpose**: Verify rendered agents match generated source (framework integrity)
- **Gate designation**: **G1** (Build Integrity — ensures rendered agent files are current)

### Branch Protection (GitHub API)
- **Primary branch**: `develop` (current HEAD)
- **Remote**: `origin` (github.com/Rushwind13/JMoria)
- **API-level rules**: Not configured in repository; would be set at GitHub organization level
- **Implication**: Integrator cannot probe/verify branch protection rules from repository; assume none configured locally, recommend verification by GI reviewer

### Deploy-on-Merge
- No CD workflow present in `.github/workflows/`
- **Finding**: Merges to `develop` do not auto-deploy; safe for agent runs
- **Gate designation**: No deploy weight; merges are information-only

### Gate Set Adopted by Host
- **G0**: (Not explicitly enforced) Code style (clang-format) — documented in Developer-Setup-Guide; CI missing
- **G1**: Render check (framework integrity) — automated GitHub Actions workflow ✓
- **G2**: (Not configured) Approval/review gates — would be GitHub API branch protection
- **G3**: (Not configured) Deployment gate — no CD workflow

**Host's actual gate capacity**: G1 only; G0 and G2–G3 are policy-documented but not machine-enforced. **Remediation for Integrator**: Document in overlays that agents MUST run clang-format before committing; cannot rely on pre-commit hook existing in fresh clones.

---

## Conventions map

### Code Style
- **clang-format**: `.clang-format` file (in repo, readable by all agents)
  - Policy: All code changes MUST pass `clang-format -i` before commit
  - Enforcement: Documented pre-commit hook template; currently not installed

### Build Conventions
- **Makefile**: Single source of truth for build targets and platform detection
- **Platform detection**: Makefile uses `uname -s` to branch macOS vs Linux
- **Output**: Single executable `jmoria` in repository root
- **Score file**: Auto-created at `Resources/Scores.txt` if missing

### Resource Data Files
- **Monster definitions**: `Resources/Monsters.txt` (custom text format)
- **Item definitions**: `Resources/Items.txt` (custom text format)
- **Tileset graphics**: `Resources/Courier.png` (OpenGL mode only)
- **Parsing**: `FileParse` utility in `src/FileParse.cpp`

### Testing Conventions
- **Feature-driven**: Cucumber feature files in `test/features/`
- **Step definitions**: C++ code in `test/features/step_definitions/`
- **Test data**: `test/fixtures/` directory
- **Test execution**: `./test/runtests.sh` (requires test dependencies installed)

### Documentation
- **Developer guide**: `doc/Developer-Setup-Guide.md` (comprehensive setup instructions)
- **Code standards**: `doc/coding-standards.md` (style & design principles)
- **Architecture notes**: `doc/_JMoria Developer's Guide.md` (dungeon generation, tile system)
- **All readable**: Fresh clones can read all conventions from these files

### Git Conventions
- **Active branch**: `develop` (tracked from origin/develop)
- **Release tags**: `0.6.0`, `0.6.1`, `0.7.0` (semantic versioning)
- **Feature branches**: Named `feat/*`, `fix/*`, `issue/*`, `phase*`
- **Commits**: Mix of feature, phase, and cleanup commits; no enforce tag pattern

---

## Guardrail register

| # | Guardrail | Traces to | Policy |
|---|-----------|-----------|--------|
| G1 | clang-format MUST be run on all staged changes before commit | `.clang-format` exists; `doc/Developer-Setup-Guide.md` documents pre-commit hook template | Agents MUST run `clang-format -i` on modified files; if pre-commit hook not installed, CI cannot enforce (see G1 gate gap below) |
| G2 | Platform compatibility MUST be maintained (Darwin/Linux/Raspberry Pi) | Makefile platform detection (`uname`), `doc/Developer-Setup-Guide.md` cross-platform section, custom instructions "Cross-Platform Compatibility" | All build script changes MUST be tested on macOS (primary) and at least one Linux variant before commit; no platform-specific hardcoding |
| G3 | Framework files (`.agentic/` core copies) are read-only; only `overlays/` are writable | `.agentic/` directory structure, `framework-lock.json`, agentic-render-check.yml verify rendered state | Agents MUST NOT edit `.agentic/contracts/`, `.agentic/roles/`, `.agentic/scripts/`, or `.agentic/registry/` directly; policy changes go in `overlays/` only |
| G4 | Rendered agent files MUST be re-generated after any overlay change | `agentic-render-check.yml` CI workflow, `render-agents.py --check`, framework-lock.json | Before committing overlay changes, run `.agentic/scripts/render-agents.py` to update `.github/agents/*` rendered files; commit both overlays and rendered files together |
| G5 | No secrets, credentials, or sensitive data in artifacts | General security policy | Do not commit API keys, database passwords, private PEM keys, or personal data; `.gitignore` contains patterns for local artifacts (scores, logs, build output) |
| G6 | Test executable must build and link on macOS with hardcoded paths | `Makefile` test flags reference `/opt/homebrew/Cellar/googletest/1.17.0/lib/`, `doc/Developer-Setup-Guide.md` lists test dependencies | Test changes (adding features or steps) MUST be validated with `./test/runtests.sh --build` on macOS before committing; hardcoded path is a technical debt but required for CI to work |

---

## Decorrelation assessment

### Vendor Reachability
- **GitHub**: ✓ Reachable (GitHub API accessible; ssh://git@github.com working)
- **Anthropic (Claude models)**: ✓ Reachable (current session via Claude Haiku 4.5)
- **OpenAI, Google, Other LLMs**: Cannot assess; assume unreachable unless configured in `registry/models.yaml`

### P5 (Plan Persistence) Satisfaction
- **Current setup**: Runs directory in-repo (`.agentic/runs/000-integration/`); supports human-readable plans
- **Assessment**: ✓ P5 **IS satisfiable** in this host
  - Integration profile, plan, intent-brief can be written to runs/
  - Contract outputs (state.yaml, etc.) persist in repo
  - Subsequent runs can read prior run metadata
- **Known limitation**: No database or sidecar state repo; all state is file-based in-repo

### Model Dispatch (registry/models.yaml)
- **Current bindings**: All roles pinned to Anthropic models (claude-fable-5, claude-sonnet-5, claude-haiku-4-5)
- **Verification**: `registry/models.yaml` specifies anthropic/* profiles only
- **Implication**: If Anthropic becomes unreachable, host falls back to alternates or pauses; ensure alternates in profiles are current

---

## Runs location

### Decision: In-Repo Runs (`.agentic/runs/`)

**Compliance posture**: JMoria is a public, open-source project (GitHub public repo) with no sensitive operational state. In-repo runs are appropriate:
- ✓ All integration/dispatch outputs are documentation-grade
- ✓ No secrets or credentials in runs
- ✓ Runs are git-tracked and auditable
- ✓ Future developers can read run history and prior decisions

**Implementation**: All runs go into `.agentic/runs/<run-id>/` (currently `000-integration/`); committed to source tree.

---

## Dispatch reality

### Operating Modes Supported
1. **Interactive operator sessions** (current) ✓
   - Integrator working interactively with Copilot CLI
   - Subsequent roles (analyst, architect, etc.) dispatched on-demand
   - Gate entries remain human-only (no automated entry points)

2. **Scheduled/self-dispatching runs** — Not yet configured
   - Could be added via GitHub Actions scheduled workflow
   - Would require dispatch manifest in a secondary run directory
   - Recommendation for future: defer to orchestrator v2; v1 not setup

3. **Autonomous orchestrator** — Not deployed
   - `roles/orchestrator.md` exists but not integrated into host CI
   - No orchestrator workflow; no entrypoint for automatic dispatch sequences
   - Safe for current phase: agents run on human dispatch only

**Host's current mode**: Interactive operator dispatch. All runs initiated by named human (Copilot CLI user); no automation yet.

---

## Gate GI record

<!-- Written ONLY by the named human approver, like any gate. -->
- approved: true
- by: Jimbo
- at: 7/14/2026 2:00pm
- notes: initial configuration; no notes

---

## Findings Summary for GI Reviewer

### Key Points
1. **Build health**: ✓ macOS ASCII build verified working; cross-platform Makefile is sound
2. **Conventions readable**: ✓ All code style and process documentation in repo; clang-format hook not yet installed but template exists
3. **CI present but incomplete**: G1 (render check) working; G0 (code style) documented but not enforced in CI — recommend pre-commit hook installation or CI gate
4. **Framework integrity**: ✓ render-agents.py check workflow in place; framework files read-only; overlays are writable
5. **No deploy risk**: No CD; merges to develop are information-only
6. **Test infrastructure**: Cucumber-CPP ready but requires googletest & cucumber-ruby setup (not in scope for Integrator)

### Recommended Actions (for GI or future runs)
- **Before first analyst dispatch**: Verify test dependencies are installable in CI environment (currently manual; see `doc/Developer-Setup-Guide.md`)
- **Before implementer deployment**: Add pre-commit hook via CI workflow or document as required setup step for developers
- **Long-term**: Consider migrating hardcoded test paths in Makefile to dynamic detection or CMake

### Decorrelation Notes
- No weakening of P5 (plan persistence); in-repo runs are safe and appropriate for this project
- All vendors (GitHub, Anthropic) presently reachable; monitor for access changes

---

**Profile completed**: 2026-07-14 | **Method**: Integrator interactive probe | **Framework version**: 1.0.70 (Copilot CLI)
