<!-- Project policy overlay. Non-comment content here is spliced into
     the ops rendered agent(s) by render-agents.py. This is the ONLY writable
     policy surface: never edit core role copies in place. -->

## Ops: JMoria Deployment & Operations

### Current Deployment Status
- **No CD pipeline**: Merges to `develop` do not auto-deploy
- **Release model**: Semantic versioning; tagged releases (`v0.6.0`, `v0.7.0`) in git
- **Distribution**: Source available on GitHub; pre-built executables not auto-published (manual if needed)

### Development & Release Branches
- **Active development**: `develop` branch (origin/develop tracked locally)
- **Release tags**: `v0.x.x` (semantic versioning); see `doc/RELEASE-*.md` for release notes
- **Feature branches**: `feat/*`, `fix/*`, `issue/*`, `phase*` prefixes

### Build Artifacts & Deployment

**Local builds**:
- Executable: `./jmoria` in repository root (created by `make` or `make ascii` or `make opengl`)
- Score file: `Resources/Scores.txt` (auto-created if missing; gitignored)
- Logs: `clockstep_log*.txt` (debug logs; gitignored)

**CI gates**:
- GitHub Actions: `agentic-render-check.yml` verifies framework rendering (passes on `develop` branch)
- No deployment gate; merges proceed if render check passes (human review via PR)

### Monitoring & Incidents

**Known operational traps**:
- Resource files missing: Game exits cleanly with error message (see `doc/Developer-Setup-Guide.md`)
- macOS Homebrew paths: If Homebrew installs libraries in different path, Makefile linking fails (update LOCAL_LIB_PATHS)
- Test path hardcoding: googletest path (`/opt/homebrew/Cellar/googletest/1.17.0/`) may differ; tests fail to link if not updated

**Troubleshooting**:
- Build fails: Check `uname` branching in Makefile; verify required libraries installed
- Tests fail to link: Check googletest path; update Makefile if needed
- Game won't start: Ensure `Resources/` directory present with `Monsters.txt`, `Items.txt`, `Courier.png` (for OpenGL)

### Operational Checklist

**Monthly or pre-release**:
- ✓ CI pipeline healthy? (`agentic-render-check.yml` green on `develop`)
- ✓ Build verified on macOS? (`make ascii` succeeds)
- ✓ Test suite passes (if dependencies installed)? (`./test/runtests.sh --build`)
- ✓ Manual smoke test: Game playable, no crash on startup
- ✓ Resource files intact: `Resources/Monsters.txt`, `Resources/Items.txt`, graphics present

**Pre-release**:
- ✓ Release branch created if needed (or tag directly on `develop`)
- ✓ Version bumped in code/docs (if applicable)
- ✓ Release notes drafted in `doc/RELEASE-x.y.z.md`
- ✓ Executable built and tested on macOS
- ✓ Tag pushed: `git tag -a vX.Y.Z -m "Release X.Y.Z"` and `git push origin vX.Y.Z`

### Framework Compliance
- ✓ `.agentic/` files untouched and read-only (managed by `render-agents.py`)
- ✓ Overlays committed alongside rendered agents (`.github/agents/*.agent.md`)
- ✓ Integration profile available for operator reference (`.agentic/runs/000-integration/`)

### No Automated Deployment
- This project is a source distribution; users build locally
- No cloud infrastructure, CDN, or auto-deployment pipeline
- Releases are GitHub tags and source archives
- Future: If deployment needed, create separate CD workflow (not in scope for this integration)
