<!-- Project policy overlay. Non-comment content here is spliced into
     the ops rendered agent(s) by render-agents.py. This is the ONLY writable
     policy surface: never edit core role copies in place. -->

## Ops Role Specifics for JMoria

### Deployment & Release Model
JMoria is an **open-source game engine**, not a deployed service. Ops responsibilities focus on:
1. **Build & packaging** (create distributable binaries)
2. **Release tagging** (GitHub releases)
3. **Dependency updates** (Homebrew, apt-get packages)
4. **Documentation & setup guides** (for developers and players)

### Build & Distribution

**Build targets:**
- `make ascii` — ASCII-only binary (smallest, headless-friendly)
- `make opengl` — OpenGL-only binary (graphical, requires X11 on Linux)
- `make` (default) — Universal binary with runtime renderer selection

**Distribution checklist:**
- [ ] Build on macOS: `make clean && make` produces `./jmoria`
- [ ] Build on Linux: Docker verification or native Linux box
- [ ] Build on Raspberry Pi: Conceptual (same Makefile, verify Debian packages available)
- [ ] Create release tarball: `jmoria-v0.x.x.tar.gz` with `jmoria` executable + `Resources/` directory
- [ ] Include: `README.md`, `Player Docs.txt`, keyboard commands
- [ ] Sign: If applicable, GPG-sign the tarball

**Release versioning:**
- Tags follow `v0.x.x` format (see git history: `v0.6.0` exists)
- Changelog: Document major features, bug fixes, breaking changes
- Platform notes: List tested platforms and known issues

### Dependency Management

**Primary dependencies:**
- C++ compiler: g++ (C++17 support required)
- Build: GNU Make
- Rendering (optional): SDL2, SDL2_image, OpenGL framework/library, ncurses
- Testing (optional): googletest, libboost, cucumber-cpp gem

**Package availability (verify before release):**
- **macOS:** Homebrew (`sdl2`, `sdl2_image`); OpenGL framework built-in
- **Linux/Debian:** apt-get (`libsdl2-dev`, `libsdl2-image-dev`, `libgl-dev`, `libncurses-dev`)
- **Raspberry Pi OS:** Debian packages (same as Linux/Debian)

**Dependency update procedure:**
- [ ] Test new version on all platforms before updating Makefile
- [ ] Document any version-specific quirks in `Developer-Setup-Guide.md`
- [ ] Notify developers of breaking changes (e.g., new build flags)

### Continuous Integration & Automation

**Current CI gate:** `.github/workflows/agentic-render-check.yml`
- Runs on push and pull_request
- Validates framework file rendering (not game build)
- Single job: `render-check` on ubuntu-latest

**Suggested enhancements (ops consideration, not required):**
1. **Build gate:** Compile on ubuntu-latest to catch Linux build issues early
2. **Test gate:** Run `make verify` on ubuntu-latest (BDD suite)
3. **Platform matrix:** Separate jobs for macOS, Linux, Raspberry Pi (if GitHub-hosted runner available)

### Monitoring & Issue Triage

**Post-release monitoring:**
- [ ] Monitor GitHub issues for crash reports
- [ ] Watch for platform-specific issues (e.g., "doesn't compile on Raspberry Pi")
- [ ] Track dependencies becoming outdated (e.g., Homebrew package removals)

**Severity levels:**
- **P0 (Critical):** Game crashes on startup, data loss, security issue
- **P1 (High):** Gameplay-breaking bug, major feature broken
- **P2 (Medium):** UI glitch, performance issue, minor feature broken
- **P3 (Low):** Cosmetic issue, nice-to-have improvement

### Documentation & Player Support

**Keep current:**
- `README.md` — Build instructions, quick start
- `Player Docs.txt` — Keyboard commands, gameplay tips
- `Developer-Setup-Guide.md` — Environment setup for developers
- `_JMoria Developer's Guide.md` — Architecture and design patterns
- Release notes: What's new in each version

**Accessibility:**
- Ensure docs are readable in fresh clone (`git clone && cat README.md`)
- Link to GitHub Wiki (if used) from README
- Provide issue template for bug reports (GitHub issue templates)

### Runbook: Release Process

1. **Preparation:**
   - [ ] Merge all features for release into develop branch
   - [ ] Update WORKLIST.txt with completed items
   - [ ] Create release notes (features, fixes, breaking changes)
   - [ ] Bump version in docs (if versioning file exists)

2. **Build verification:**
   - [ ] `make clean && make ascii` on macOS
   - [ ] `make clean && make opengl` on macOS
   - [ ] `make clean && make` on macOS (both)
   - [ ] Test on Linux via Docker: `docker run -it ubuntu:latest /bin/bash` then install deps and build
   - [ ] Spot-check gameplay: new features work, no regressions

3. **Release:**
   - [ ] Tag: `git tag v0.x.x && git push origin v0.x.x`
   - [ ] GitHub release: Upload `jmoria-v0.x.x.tar.gz` executable + Resources/ directory
   - [ ] Announce: Update README.md if needed; post release notes

4. **Post-release:**
   - [ ] Monitor for crash reports (first 24 hours critical)
   - [ ] Prepare hotfix branch if P0 issues found
   - [ ] Plan next release features in WORKLIST.txt

### Known Ops Constraints
- No deployment pipeline (this is source code, not a service)
- Release is manual git tag + GitHub release upload
- No automated scaling, load balancing, or infrastructure changes
- Platform testing responsibility shared with developers (no dedicated CI/CD platform farm)
