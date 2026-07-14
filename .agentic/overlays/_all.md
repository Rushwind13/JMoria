<!-- Project policy overlay. Non-comment content here is spliced into
     every rendered agent(s) by render-agents.py. This is the ONLY writable
     policy surface: never edit core role copies in place. -->

# JMoria Project Layer: Universal Guardrails

## Code Style & Format (G0)
- **clang-format**: All code changes MUST pass `clang-format -i` before staging
  - `.clang-format` configuration is in repo root (read-only)
  - Run `clang-format -i <file>` on any modified `.cpp` or `.h` files
  - Pre-commit hook template exists in `doc/Developer-Setup-Guide.md` (currently not installed)
  - **CI gap**: G0 is not yet automated; enforcement is manual review until hook is installed

## Platform Compatibility (G2)
- JMoria supports **three platforms**: macOS (primary), Linux/Ubuntu, Raspberry Pi OS
- All build scripts and code MUST remain compatible with all three
- **Makefile platform detection**: Uses `uname -s` to branch Darwin vs Linux
- **Tested build modes**:
  - `make ascii` — terminal-only (ncurses); fastest
  - `make opengl` — SDL2+OpenGL (graphics); requires framework on macOS
  - `make` (default) — both renderers, runtime selection
- **If you modify Makefile, build scripts, or dependencies**:
  - Test `make ascii` on macOS (required)
  - Test on Linux or Raspberry Pi if feasible (recommended; if not, document testing assumptions)
  - Verify `uname` branching logic still works after changes

## Framework Integrity (G1)
- JMoria uses the agentic framework for AI-assisted development
- `.agentic/` contains framework core files (read-only); editing them breaks subsequent runs
- **Only writable files**: `.agentic/overlays/` (project policy) and `.agentic/registry/models.yaml` (vendor bindings)
- **Before committing overlay changes**:
  - Run `.agentic/scripts/render-agents.py` to regenerate `.github/agents/*.agent.md`
  - Commit both overlay changes AND generated agent files
  - CI workflow `agentic-render-check.yml` will verify consistency

## Resource Data Files (Convention)
- Monster definitions: `Resources/Monsters.txt` (custom format, parsed by `CDataFile::ReadMonster()`)
- Item definitions: `Resources/Items.txt` (custom format, parsed by `CDataFile::ReadItem()`)
- **To add a new monster or item flavor**: Edit the `.txt` file only
- **To add a new type**: Edit `.txt` file + add constant to `src/Constants.h` + add emoji to `MonIds`/`ItemIds` in `src/Monster.cpp` or `src/Item.cpp`
- Utility scripts available: `scripts/find_monster.sh`, `scripts/list_item.sh`, etc.

## Testing
- **Framework**: Cucumber-CPP + GoogleTest
- **Location**: `test/features/` (feature files) + `test/features/step_definitions/` (C++ steps)
- **Run tests**: `make clean ascii test; cd test; ./runtests.sh` (requires test dependencies)
- **Test dependencies** (macOS): `brew install googletest cucumber-cpp`; `sudo gem install cucumber -v 7.1.0`
- **Known constraint**: Hardcoded googletest path in Makefile (`/opt/homebrew/Cellar/googletest/1.17.0/`); works on macOS with Homebrew; Linux may differ
- **If adding test files**: Ensure they build and link before committing

## Security
- **No secrets in artifacts**: Do not commit API keys, credentials, or personal data
- **Gitignore**: Covers local artifacts (scores, logs, temp files); review `.gitignore` if adding new types
- **.agentic/ state**: Safe to commit; contains no secrets, only framework and run artifacts

## Documentation
- **Developer guide**: `doc/Developer-Setup-Guide.md` (setup, dependencies, build, hooks)
- **Code standards**: `doc/coding-standards.md` (style guidelines)
- **Architecture**: `doc/_JMoria Developer's Guide.md` (dungeon, AI, tile system)
- **Custom instructions**: `.github/copilot-instructions.md` (Copilot context for this project)

## Branches & Releases
- **Active branch**: `develop` (tracked from origin/develop)
- **Releases**: Semantic versioning (0.6.x, 0.7.x); tag as `v<version>`
- **Feature branches**: Use `feat/*`, `fix/*`, `issue/*` prefixes
- **No branch protection configured**: Merges to develop are allowed; rely on review via PR (human gate)

## Abbreviations
- **G0–G3**: Gate set (code style, build integrity, approval/review, deploy)
- **P5**: Plan persistence (agentic framework requirement; satisfied by in-repo runs)
- **CI**: GitHub Actions workflows (`.github/workflows/`)
- **clang-format**: LLVM code formatter; version 21.1.8 available on macOS
