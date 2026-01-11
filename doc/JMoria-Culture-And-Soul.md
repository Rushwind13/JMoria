# JMoria: Culture and Soul

## What is JMoria?

JMoria is a from-scratch roguelike game, an homage to the classic IMoria (1983) and the Moria/Angband family of games. It aims to capture the spirit of dungeon-crawling adventure games from the golden age of roguelikes.

This is not a commercial project. It's a labor of love—a personal project that has been worked on sporadically across decades, whenever inspiration strikes and time permits.

---

## Project History

| Year | Milestone |
|------|-----------|
| ~2003 | Project started (earliest WORKLIST entries: `4/10/03`) |
| 2005 | Active development, dungeon generation work (`1.1.05`) |
| 2006 | Gameplay focus, flipcode research, Windows/VSS era (`1.27.06`) |
| 2017 | Ported to macOS, migrated from VSS to GitHub, SDL2 adoption |
| 2024-2026 | AI observability features, testing infrastructure, continued gameplay |

The project has survived multiple operating systems (Windows, PS2 experiments, macOS, Linux), version control systems (VSS to Git), and technology eras.

---

## Guiding Principles

### 1. Simplicity Over Sophistication

The best code is code that a tired developer at midnight can understand. Clever solutions create maintenance burdens. Straightforward solutions last decades.

### 2. Working Code Over Perfect Code

A feature that works today is better than an elegant abstraction that might work someday. Ship it, play it, improve it later if needed.

### 3. Respect the Vintage

Roguelikes have a distinct aesthetic: `@` is the player, `d` is a dragon, `hjklyubn` moves you around. This isn't legacy baggage—it's the soul of the genre. Honor it.

### 4. Personal Satisfaction Over External Validation

This project exists because it's fun to work on. There are no deadlines, no sprints, no stakeholders. Progress happens when it happens.

---

## Technology Philosophy

### Preferred Approaches

- **C++ with manual memory management** - No smart pointer soup needed for a single-player game
- **Makefiles** - Simple, portable, been working since the 1970s
- **Text-based data files** - Monsters.txt, Items.txt, simple parseable formats
- **SDL2** - Cross-platform graphics without framework lock-in
- **ASCII tileset** - True to roguelike tradition

### Era-Appropriate Thinking

When considering how to implement something, ask: "How would this have been done in 1990?" That's often the right answer. Not because modern approaches are wrong, but because:

1. Simpler solutions have fewer bugs
2. This project should feel like a roguelike, not a modern game engine
3. Dependencies become liabilities over decades

---

## Anti-Patterns

### Technology Anti-Patterns

| Don't | Why |
|-------|-----|
| JSON/YAML/TOML for game data | Use simple text formats (colon-delimited, INI-style). Monsters.txt is the model. |
| Package managers (npm, pip, cargo) | External dependencies rot. Vendor what you need or write it yourself. |
| Build systems beyond Make | CMake, Bazel, Meson add complexity for marginal benefit here. |
| Frameworks that impose structure | Prefer libraries you call over frameworks that call you. |
| Template metaprogramming | If the code requires a PhD to read, it's wrong for this project. |
| Abstract base classes for everything | Concrete classes are fine. YAGNI. |

### Design Anti-Patterns

| Don't | Why |
|-------|-----|
| Premature abstraction | "We might need this later" is how projects die under their own weight. |
| Factory factories | One factory is plenty. Zero is often better. |
| Dependency injection frameworks | Constructor arguments work fine. |
| Configuration over convention | Hard-code it. Change it when you need to. |
| Enterprise patterns in a single-player game | No microservices. No message queues. No distributed anything. |

### What's Acceptable

Some modern tooling is worth the tradeoff:

- **SDL2** - Necessary for cross-platform graphics
- **Google Test / Cucumber** - Testing infrastructure, isolated in `/test`
- **AI logging as JSONL** - Integration with AI tooling (not game data, just observability)
- **Git / GitHub** - Version control is non-negotiable

---

## For AI Assistants

If you're an AI helping with this project, keep these guidelines in mind:

### Do

- Suggest the simplest solution that works
- Respect existing patterns in the codebase
- Understand that "old-fashioned" code is intentional, not ignorance
- Keep changes minimal and focused
- Ask before adding dependencies

### Don't

- Suggest "modern best practices" that add complexity
- Refactor working code to be more "idiomatic"
- Add error handling for scenarios that can't happen
- Create abstractions for one-time operations
- Recommend libraries when 20 lines of code suffices
- Add type annotations, docstrings, or comments to code you didn't change

### Remember

This project has been around since 2003 and will hopefully be around for decades more. Changes should be made with that timescale in mind. The goal is a fun roguelike game, not a showcase of software engineering techniques.

---

## The Soul of JMoria

At its core, JMoria is about:

- Exploring procedurally generated dungeons
- Fighting monsters with ASCII representation
- Finding treasure and growing stronger
- Dying a lot (permadeath is part of the experience)
- Having fun

Everything else—the code, the architecture, the tooling—exists in service of that experience. When in doubt, ask: "Does this make the game more fun to play or more fun to work on?" If the answer is no, it's probably not needed.

---

*"You are in a maze of twisty little passages, all alike."*