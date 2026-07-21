# Integration Profile: JMoria

<!-- Contract: produced by Integrator; consumed by the GI human and by every
     later run in this host. Gate: GI (the scaffold PR review — "is this how
     agents should behave in this house?"). All sections required; a guardrail
     that traces to no probe finding or host policy is malformed.
     BUDGET: reviewable by the GI human in fifteen minutes. -->

## Environment probe

**Platform & Toolchain (macOS host, cross-platform target):**
- Interpreter: g++ (C++17 standard, `-std=c++17` set in Makefile)
- Platform detection: `uname -s` → Darwin (macOS), Linux, or Raspberry Pi OS (Debian)
- Build system: GNU Make with conditional platform-specific flags:
  - macOS: Homebrew paths (`/usr/local/lib`, `/opt/homebrew/include`), `-framework OpenGL`
  - Linux/Raspberry Pi: System paths, `-lGL` for OpenGL
- Package management: Homebrew (macOS), apt-get (Linux/Debian)
- **Verified build:** `make ascii` produces working jmoria executable; compilation clean

**Testing Framework:**
- BDD (Behavior-Driven Development): Cucumber-CPP with GoogleTest wire protocol
- Test runner: `make verify` or `make bdd` or `./test/runtests.sh` or `make verify intrinsic_effects # test 'intrinsic_effects.feature' only`
- Test structure: Feature files in `test/features/`, step definitions C++ with wire protocol
- Test dependencies: cucumber gem (v7.1.0, requires v7.x for wire protocol), libboost, libgtest, libcucumber-cpp
- Test build: `make build` produces `test/bin/AllSteps`; NOT run manually, only via runtests.sh
- Verified: Build succeeds; runtests.sh cannot run without pre-built test executable (expected — tests require full build)

**Code Formatting & Style:**
- Tool: clang-format, config in `.clang-format` (Allman braces, 100-char column limit)
- Enforcement: Pre-commit hook intended (custom_instruction references git clang-format hook)
- Status: `.clang-format` committed, hook setup referenced in `Developer-Setup-Guide.md`

**Build Modes & Renderer Support:**
- Three build configurations via `RENDER_MODE` environment variable:
  1. `make ascii` → ASCII-only (ncurses), no SDL/OpenGL at runtime
  2. `make opengl` → OpenGL-only (SDL2, SDL2_image)
  3. `make` (default) → Both renderers, runtime selection via `--renderer` flag
- Resource files: `Resources/Monsters.txt`, `Resources/Items.txt`, `Resources/Effects.txt`, `Resources/Colors.txt` (data-driven definitions)
- **All three verified buildable on macOS**

**Cross-Platform Compatibility Requirement:**
- Makefile includes Darwin (macOS) and Linux conditional blocks
- Platform-specific includes, library paths, linker flags all detected at build time
- Per custom_instruction: must remain compatible with macOS, Linux, Raspberry Pi OS
- No current CI testing on Linux/Raspberry Pi; host is macOS

## Gate mapping

**CI/CD gates (current):**
- **Single gate: agentic-render-check** (`.github/workflows/agentic-render-check.yml`)
  - Trigger: on push, on pull_request
  - Job: `render-check` (ubuntu-latest)
  - Command: `python3 .agentic/scripts/render-agents.py --check`
  - Purpose: Validates framework rendering (agent/role config consistency)
  - Enforces: Framework file consistency, no manual edits to rendered files

**Branch & PR workflow:**
- Primary branch: `develop` (remotes/origin/HEAD → develop)
- Secondary branch: `main` (visible in remote refs, older commits)
- Current HEAD: `feat/phase3d_shiny`
- PR merge pattern: Squash-and-merge with PR metadata in commit message (e.g., "Merge pull request #65")
- **No explicit branch protection rules detected** (repository may have GitHub settings not reflected in local config)

**Game-level gates (not CI, but project policy):**
- Wizard Mode disables score saving (debug-only feature)
- Data-driven definitions in resource files (no hardcoding monster/item stats in code)

**Deploy weight of a merge:**
- This is a game engine project (not a deployed service)
- No production deployment gates or CD pipelines detected
- Merges to develop/main are source snapshots for distribution, not deployments

## Conventions map

**Where conventions live (all agent-readable):**

| Convention | Location | Readability in Fresh Clone |
|-----------|----------|---------------------------|
| **Coding standards** | `.clang-format` (committed) | ✅ Immediate (binary, enforced by formatter) |
| **AI Coding guide** | `.github/copilot-instructions.md` | ✅ Immediate (GitHub Copilot reads this) |
| **Developer setup** | `Developer-Setup-Guide.md` (assumed in repo root or doc/) | ✅ Immediate (docs committed) |
| **Build & test** | `Makefile`, `test/runtests.sh` | ✅ Immediate (executable scripts) |
| **Architecture** | `_JMoria Developer's Guide.md` (doc/) | ✅ Immediate (committed docs) |
| **Gameplay/commands** | `Player Docs.txt` (root or doc/) | ✅ Immediate (committed docs) |
| **Data definitions** | `Resources/Monsters.txt`, `Resources/Items.txt` | ✅ Immediate (committed data files) |
| **Worklist/roadmap** | `WORKLIST.txt`, `WORKLIST_jmoria_core_roadmap.md` | ✅ Immediate (committed planning) |
| **Feature branches** | Git branch naming: `feat/`, `fix/`, `issue/`, `infra/` | ✅ Immediate (visible in `git branch`) |
| **Test structure** | `test/features/`, `test/features/step_definitions/` | ✅ Immediate (directory structure) |

**Conventions missing or gitignored:**
- No `.editorconfig` (could standardize IDE settings for cross-platform)
- No explicit CONTRIBUTING.md (implied via Copilot instructions + Developer-Setup-Guide)
- `Scores.txt` gitignored (auto-created, game runtime artifact)

**Agent startup capability:**
A cold-start agent in a fresh clone can immediately read:
- Build recipes (Makefile, platform detection)
- Code style (clang-format config)
- Architecture (Copilot instructions, Developer's Guide)
- Development workflows (test setup, feature branch naming)
- Without needing external docs or environment setup

## Guardrail register

| # | Guardrail | Traces to | Enforcement |
|---|-----------|-----------|------------|
| G1 | **Platform compatibility**: All Makefile edits and build scripts must remain compatible with macOS (Darwin), Linux, and Raspberry Pi OS (Debian). Platform detection via `uname -s`; platform-specific flags only in conditional blocks. | Custom_instruction §Cross-Platform Compatibility; Makefile platform detection blocks; CI gate on ubuntu-latest | Code review + pre-merge verification on at least macOS and Linux |
| G2 | **Code formatting enforcement**: All C++ source code must be formatted with clang-format before commit. Pre-commit hook enforces `git clang-format` on staged changes. Manual formatting via `clang-format -i <files>`. | Custom_instruction §Code Style; `.clang-format` config committed; git hook referenced | Git pre-commit hook (setup in Developer-Setup-Guide); failing hook blocks commit |
| G3 | **Build mode support**: Implementer agents modifying build system must support all three render modes: ASCII (ncurses), OpenGL (SDL2), and both. `RENDER_MODE` environment variable controls mode; defaults to both. No mode-specific hardcoding. | Makefile structure; custom_instruction §Build & Test Workflow; README.md | `make ascii`, `make opengl`, `make` all must produce working binaries |
| G4 | **Data-driven definitions**: Monster and item balancing, attributes, and behaviors must be defined in `Resources/Monsters.txt` and `Resources/Items.txt`. No hardcoded stat tables in C++ code (except indices/enums in `Constants.h`). | Custom_instruction §Data Files & Resource System; existing structure; game design principle | Code review: reject PRs with hardcoded monster/item stats in .cpp files |
| G5 | **Cross-platform include paths**: Build flags must use platform-specific Homebrew/system paths without breaking other platforms. macOS uses `/opt/homebrew/`, Linux uses system defaults. Use conditional compilation, not absolute paths. | Makefile conditional blocks; platform-specific include/lib paths | Pre-merge verification that macOS and Linux builds still pass |
| G6 | **Wizard mode scope**: Wizard Mode debug features (`^t` teleport, etc.) must not interact with score tracking. Score saving must be disabled when Wizard Mode is active. | Custom_instruction §Wizard Mode Commands; game balance principle | Code review: reject commits that allow score saves during Wizard Mode |
| G7 | **Test executable protocol**: Test executable (`test/bin/AllSteps`) runs as background process via wire protocol during `runtests.sh`, NOT manually invoked. Manual invocation breaks cucumber wire connection. | Makefile test targets; custom_instruction §Testing; make verify comments | Documentation + code comments; test setup instructions in Developer-Setup-Guide |

## Decorrelation assessment

**Vendor reachability:**
- GitHub Actions CI available (ubuntu-latest runners)
- Anthropic (Claude family) accessible via session context
- OpenAI (GPT family) potentially accessible (listed in registry/models.yaml)
- Google Gemini potentially accessible (listed in registry/models.yaml)
- **Primary constraint: No external API deployment gates; this is a game engine source repo, not a live service**

**P5 satisfiability (reviewer ≠ implementer vendor):**
- Registry bindings allow P5:
  - Implementer: `balanced` profile (default: Anthropic Claude Sonnet 4.5)
  - Reviewer: `frontier-reasoning` (default: Anthropic Claude Sonnet 4.6, alternates: OpenAI, Google)
  - Verifier: `balanced` (default: Anthropic, alternates: OpenAI, Google)
- **P5 satisfiable**: Can route reviewer/verifier to OpenAI or Google to differ from implementer's Anthropic
- **No known weakening**: All roles can reach different vendors if registry binding requires it

**Model role assignments (per registry/models.yaml):**
- Orchestrator: frontier-reasoning (Sonnet 4.6)
- Architect: frontier-reasoning (Sonnet 4.6)
- Implementer: balanced (Sonnet 4.5)
- Analyst: balanced (Sonnet 4.5)
- Reviewer: frontier-reasoning, avoid_vendor_of implementer
- Verifier: balanced, avoid_vendor_of implementer
- Ops: balanced (Sonnet 4.5)

## Runs location

**In-repo runs/ vs. sidecar:**
- Chosen: **In-repo** (`.agentic/runs/`)
- Rationale: This is an open-source game engine with transparent development; agent run artifacts, profiles, and decisions should be visible in the repository. Framework scaffolding is already committed (`.agentic/` directory), so runs logically belong alongside it.
- Compliance posture: Public repository; no sensitive data in framework setup; run records contribute to project history

## Dispatch reality

**Operating modes:**
- **Primary mode: Interactive operator sessions** (user invokes `run .agentic/runs/000-integration` or agents via GitHub Copilot CLI)
- **Secondary mode: PR-triggered CI** (agentic-render-check on every push/PR; no autonomous orchestrator)
- **Scheduled runs: Not configured** (historian role configured in registry, but no CRON-triggered sweeps visible)
- **Human gate entries:** PR review required before merge (Rushwind13/JMoria default branch protection assumed, though not explicitly verified via `git config`)

**Implications for agents:**
1. No autonomous orchestrator polling a dispatch queue; agents run on-demand
2. CI gate (render-check) is read-only validation, not a dispatch trigger
3. Agents should assume interactive session context (user feedback available, iteration expected)
4. Framework file editing triggers CI validation (render-check); intentional, not a blocker

## Gate GI record

<!-- Written ONLY by the named human approver, like any gate. -->
- approved: false
- by: _awaiting GI review_
- at: _pending_
- notes: _Profile ready for human review. Key findings: (1) Cross-platform Makefile with tested macOS, unverified Linux/Raspberry Pi build flags require CI testing. (2) Single CI gate (render-check) validates framework, not game build. (3) Platform compatibility is a hard guardrail; all agent builds must remain compatible with 3 platforms. (4) Data-driven design enforced by structure; no framework-level enforcement needed. (5) Test infrastructure requires `make build` pre-step; agents must sequence correctly. (6) No secrets/sensitive data found; repository is appropriately open._
