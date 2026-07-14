#!/usr/bin/env python3
"""integrate.py — vendored-copy integration of the framework into a host repo.

The mechanical half of docs/INTEGRATION.md: `init` copies the taken subset of
the copy manifest into a host repository, seeds the project-owned layers,
writes the framework lockfile, renders adapter agents, and wires the
render-staleness CI check. `validate` re-proves the static integration
invariants. `fork` records a deliberate divergence of a core file before you
edit it.

Usage (from a pinned framework release checkout or tarball):
    python3 <framework>/scripts/integrate.py init <target-repo> \
        --provenance redistribute|private \
        [--take all|sdlc|<file,file,...>] [--layout prefixed|root] \
        [--prefix .agentic] [--adapters auto|<name,name,...>]

Usage (from the vendored copy inside a host repo):
    python3 scripts/integrate.py validate [<target-repo>] [--prefix .agentic]
    python3 scripts/integrate.py fork <core-file> --reason "why" [...]

Stdlib-only and Python 3.9-compatible on purpose, same constraint as the
renderer and for the same reason: this tool runs *before* the environment
probe has fixed anything (INTEGRATION.md §8).
"""
import argparse
import hashlib
import json
import re
import shutil
import subprocess
import sys
from datetime import date
from pathlib import Path

TOOL_VERSION = "0.1"
SOURCE = Path(__file__).resolve().parent.parent
PROVENANCE_START = "<!-- agentic-framework-provenance:start -->"
PROVENANCE_END = "<!-- agentic-framework-provenance:end -->"


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def load_json(path):
    return json.loads(Path(path).read_text(encoding="utf-8"))


def load_copy_manifest(root):
    path = root / "scripts" / "copy-manifest.json"
    if not path.exists():
        raise SystemExit("no copy manifest at %s — is this a framework release?" % path)
    return load_json(path)


def resolve_take(manifest, take):
    """Resolve --take into the ordered list of core-layer files to copy."""
    offered = list(manifest["always"])
    for group in sorted(manifest["groups"]):
        for item in manifest["groups"][group]:
            if item not in offered:
                offered.append(item)
    if take == "all":
        return offered
    if take in manifest["groups"]:
        chosen = list(manifest["always"]) + [
            f for f in manifest["groups"][take] if f not in manifest["always"]
        ]
        return chosen
    explicit = [item.strip() for item in take.split(",") if item.strip()]
    unknown = [f for f in explicit if f not in offered]
    if unknown:
        raise SystemExit(
            "not in the copy manifest: %s\n(offered: %s)"
            % (", ".join(unknown), ", ".join(offered))
        )
    return list(manifest["always"]) + [f for f in explicit if f not in manifest["always"]]


def detect_adapters(target):
    adapters = []
    if (target / ".claude").is_dir() or (target / "CLAUDE.md").exists():
        adapters.append("claude-code")
    if (target / ".github" / "agents").is_dir() or (
        target / ".github" / "copilot-instructions.md"
    ).exists():
        adapters.append("copilot-cli")
    return adapters


def source_info():
    """Pin the source: git metadata when available, tarball placeholders otherwise."""
    info = {"repo": None, "ref": "unknown", "version": "unreleased"}
    git = ["git", "-C", str(SOURCE)]
    try:
        info["ref"] = subprocess.run(
            git + ["rev-parse", "HEAD"], capture_output=True, text=True, check=True
        ).stdout.strip()
        remote = subprocess.run(
            git + ["remote", "get-url", "origin"], capture_output=True, text=True
        )
        if remote.returncode == 0 and remote.stdout.strip():
            info["repo"] = remote.stdout.strip()
        tag = subprocess.run(
            git + ["describe", "--tags", "--exact-match"], capture_output=True, text=True
        )
        if tag.returncode == 0 and tag.stdout.strip():
            info["ref"] = tag.stdout.strip()
            info["version"] = tag.stdout.strip().lstrip("v")
    except (OSError, subprocess.CalledProcessError):
        pass
    return info


def roles_in(taken):
    return [Path(f).stem for f in taken if f.startswith("roles/")]


def renderer_rel_path(layout, prefix):
    if layout == "prefixed":
        return "%s/scripts/render-agents.py" % prefix
    return "scripts/render-agents.py"


def write_provenance(target, prefix_dir, mode, lock_rel):
    """Provenance in two modes (INTEGRATION.md §5); the managed NOTICE section
    is idempotent — rewritten between markers on every init."""
    license_src = SOURCE / "LICENSE.md"
    if license_src.exists():
        shutil.copy2(str(license_src), str(prefix_dir / "LICENSE.framework.md"))
    if mode == "redistribute":
        section = (
            "This repository includes files derived from the Agentic Development\n"
            "System framework, licensed under the Apache License 2.0. The framework\n"
            "license text is kept at %s/LICENSE.framework.md; the derived files are\n"
            "enumerated in %s." % (prefix_dir.name, lock_rel)
        )
    else:
        section = (
            "This repository is private and does not redistribute. It contains\n"
            "framework-derived files enumerated in %s, used under the Apache\n"
            "License 2.0; the framework license text is kept at\n"
            "%s/LICENSE.framework.md. No repo-root license applies to these files."
            % (lock_rel, prefix_dir.name)
        )
    block = "%s\n%s\n%s" % (PROVENANCE_START, section, PROVENANCE_END)
    notice = target / "NOTICE.md"
    if notice.exists():
        text = notice.read_text(encoding="utf-8")
        if PROVENANCE_START in text and PROVENANCE_END in text:
            head, _, rest = text.partition(PROVENANCE_START)
            _, _, tail = rest.partition(PROVENANCE_END)
            text = head + block + tail
        else:
            text = text.rstrip() + "\n\n" + block + "\n"
    else:
        text = "# NOTICE\n\n" + block + "\n"
    notice.write_text(text, encoding="utf-8")


def prefix_readme(prefix_dir, info):
    body = (
        "# Framework metadata\n\n"
        "This directory pins this repository's integration of the Agentic\n"
        "Development System (source: %s, ref %s). `framework-lock.json` is the\n"
        "authoritative record of what is framework core versus instance-local;\n"
        "`upstream/` retains the pristine base of any recorded fork. Do not edit\n"
        "core-layer copies in place — extend via `overlays/` or record a fork:\n"
        "`python3 scripts/integrate.py fork <file> --reason \"...\"`.\n"
        % (info["repo"] or "<framework repo>", info["ref"][:12])
    )
    (prefix_dir / "README.md").write_text(body, encoding="utf-8")


def wire_ci(target, layout, prefix):
    workflows = target / ".github" / "workflows"
    workflow = workflows / "agentic-render-check.yml"
    if workflow.exists():
        return
    workflows.mkdir(parents=True, exist_ok=True)
    workflow.write_text(
        "name: agentic-render-check\n"
        "on: [push, pull_request]\n"
        "jobs:\n"
        "  render-check:\n"
        "    runs-on: ubuntu-latest\n"
        "    steps:\n"
        "      - uses: actions/checkout@v4\n"
        "      - run: python3 %s --check\n" % renderer_rel_path(layout, prefix),
        encoding="utf-8",
    )


def run_renderer(core_root, check=False):
    cmd = [sys.executable, str(core_root / "scripts" / "render-agents.py")]
    if check:
        cmd.append("--check")
    return subprocess.run(cmd, capture_output=True, text=True)


def cmd_init(args):
    target = Path(args.target).resolve()
    if not target.is_dir():
        raise SystemExit("target %s is not a directory" % target)
    if not (target / ".git").exists():
        print("note: %s is not a git repository; proceeding anyway" % target)

    prefix_dir = target / args.prefix
    lock_path = prefix_dir / "framework-lock.json"
    old_lock = load_json(lock_path) if lock_path.exists() else None
    if old_lock:
        for flag, key in (("layout", "layout"), ("prefix", "prefix")):
            if getattr(args, flag) != old_lock[key]:
                raise SystemExit(
                    "existing lock records %s=%s; re-run with the same value or "
                    "remove the lock deliberately" % (key, old_lock[key])
                )

    manifest = load_copy_manifest(SOURCE)
    taken = resolve_take(manifest, args.take) if not old_lock else old_lock["taken"]
    forks = old_lock["forks"] if old_lock else {}
    core_root = prefix_dir if args.layout == "prefixed" else target

    adapters = (
        [a.strip() for a in args.adapters.split(",") if a.strip()]
        if args.adapters != "auto"
        else detect_adapters(target)
    )
    if not adapters:
        adapters = ["claude-code"]
        print("note: no runner detected; defaulting to the claude-code adapter")
    for adapter in adapters:
        if not (SOURCE / "adapters" / adapter / "manifest.json").exists():
            raise SystemExit("unknown adapter '%s'" % adapter)

    # Core layer: copy the taken subset. Drifted unforked copies are an error,
    # never silently clobbered; recorded forks are never touched.
    prefix_dir.mkdir(parents=True, exist_ok=True)
    drifted = []
    lock_files = {}
    for rel in taken:
        dest = core_root / rel
        host_rel = dest.relative_to(target).as_posix()
        if host_rel in forks:
            lock_files[host_rel] = old_lock["files"].get(host_rel, "")
            continue
        if (
            dest.exists()
            and old_lock
            and old_lock["files"].get(host_rel)
            and sha256(dest) != old_lock["files"][host_rel]
        ):
            drifted.append(host_rel)
            continue
        dest.parent.mkdir(parents=True, exist_ok=True)
        src = SOURCE / rel
        if src.resolve() != dest.resolve():
            shutil.copy2(str(src), str(dest))
        lock_files[host_rel] = sha256(dest)
    if drifted:
        raise SystemExit(
            "core-layer files edited in place (lock checksum mismatch):\n  %s\n"
            "Move the change to overlays/, or record each as a fork:\n"
            "  python3 %s fork <file> --reason \"...\"\n"
            "then re-run init." % ("\n  ".join(drifted), Path(__file__).name)
        )

    # Seeded layer: template on first init, project-owned after.
    registry_dest = core_root / "registry" / "models.yaml"
    if not registry_dest.exists():
        registry_dest.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(str(SOURCE / "registry" / "models.yaml"), str(registry_dest))
    for adapter in adapters:
        man_dest = core_root / "adapters" / adapter / "manifest.json"
        if not man_dest.exists():
            man_dest.parent.mkdir(parents=True, exist_ok=True)
            man = load_json(SOURCE / "adapters" / adapter / "manifest.json")
            man["roles"] = [r for r in man["roles"] if r in roles_in(taken)]
            if "integrator" in roles_in(taken) and "integrator" not in man["roles"]:
                man["roles"].append("integrator")
            if args.layout == "prefixed":
                man["output_dir"] = "../" + man["output_dir"]
            man_dest.write_text(
                json.dumps(man, indent=2) + "\n", encoding="utf-8"
            )

    # Project layer: overlay stubs for the taken roles only.
    overlays = core_root / "overlays"
    overlays.mkdir(parents=True, exist_ok=True)
    stub = (
        "<!-- Project policy overlay. Non-comment content here is spliced into\n"
        "     %s rendered agent(s) by render-agents.py. This is the ONLY writable\n"
        "     policy surface: never edit core role copies in place. -->\n"
    )
    if not (overlays / "_all.md").exists():
        (overlays / "_all.md").write_text(stub % "every", encoding="utf-8")
    for role in roles_in(taken):
        role_stub = overlays / ("%s.md" % role)
        if not role_stub.exists():
            role_stub.write_text(stub % ("the %s" % role), encoding="utf-8")

    # The integration run directory, ready for the Integrator dispatch.
    run_dir = core_root / "runs" / "000-integration"
    run_dir.mkdir(parents=True, exist_ok=True)

    info = source_info()
    lock_rel = "%s/framework-lock.json" % args.prefix
    write_provenance(target, prefix_dir, args.provenance, lock_rel)
    prefix_readme(prefix_dir, info)
    wire_ci(target, args.layout, args.prefix)

    render = run_renderer(core_root)
    if render.returncode != 0:
        raise SystemExit("render failed:\n%s%s" % (render.stdout, render.stderr))

    lock = {
        "source": info,
        "integrated_at": date.today().isoformat(),
        "method": "integrate.py v%s" % TOOL_VERSION,
        "adapters_rendered": adapters,
        "taken": taken,
        "files": lock_files,
        "forks": forks,
        "instance_layer": old_lock["instance_layer"] if old_lock else [],
        "provenance_mode": args.provenance,
        "layout": args.layout,
        "prefix": args.prefix,
    }
    lock_path.write_text(json.dumps(lock, indent=2) + "\n", encoding="utf-8")

    validate_cmd = "python3 %s validate" % renderer_rel_path(
        args.layout, args.prefix
    ).replace("render-agents.py", "integrate.py")
    print("integrated %s files into %s (%s layout, %s adapters: %s)" % (
        len(lock_files), target, args.layout,
        len(adapters), ", ".join(adapters),
    ))
    print("\nNext, from %s:" % target)
    print("  1. dispatch the Integrator in your runner:")
    print("       Use the integrator subagent for run runs/000-integration,")
    print("       producing integration-profile.md per contracts/integration-profile.md.")
    print("  2. review its integration-profile.md and overlays (gate GI), then:")
    print("       %s" % validate_cmd)
    print("  3. open the scaffold PR.")
    return 0


def find_lock(target, prefix):
    lock_path = target / prefix / "framework-lock.json"
    if not lock_path.exists():
        raise SystemExit("no lock at %s — run init first (or pass the host repo path)" % lock_path)
    return lock_path


def cmd_validate(args):
    target = Path(args.target).resolve()
    lock_path = find_lock(target, args.prefix)
    lock = load_json(lock_path)
    failures = []

    schema = load_json(SOURCE / "scripts" / "framework-lock.schema.json")
    for key in schema["required"]:
        if key not in lock:
            failures.append("lock: missing required field '%s'" % key)
    if lock.get("provenance_mode") not in ("redistribute", "private"):
        failures.append("lock: provenance_mode must be redistribute|private")

    prefix_dir = target / lock.get("prefix", args.prefix)
    for host_rel, checksum in lock.get("files", {}).items():
        if host_rel in lock.get("forks", {}):
            continue
        path = target / host_rel
        if not path.exists():
            failures.append("missing core file: %s" % host_rel)
        elif sha256(path) != checksum:
            failures.append(
                "edited in place: %s (move to overlays/ or record a fork)" % host_rel
            )
    for host_rel, fork in lock.get("forks", {}).items():
        base = prefix_dir / "upstream" / host_rel
        if not base.exists():
            failures.append("fork %s: retained upstream copy missing at %s"
                            % (host_rel, base.relative_to(target).as_posix()))
        elif sha256(base) != fork["base_sha256"]:
            failures.append("fork %s: retained upstream copy no longer matches its "
                            "recorded base" % host_rel)

    core_root = prefix_dir if lock.get("layout") == "prefixed" else target
    render = run_renderer(core_root, check=True)
    if render.returncode != 0:
        failures.append("stale rendered agents (run: python3 %s)"
                        % renderer_rel_path(lock.get("layout", "root"),
                                            lock.get("prefix", args.prefix)))

    notice = target / "NOTICE.md"
    if not notice.exists() or PROVENANCE_START not in notice.read_text(encoding="utf-8"):
        failures.append("provenance: NOTICE.md missing its framework section")
    if not (prefix_dir / "LICENSE.framework.md").exists():
        failures.append("provenance: %s/LICENSE.framework.md missing" % prefix_dir.name)

    overlays = core_root / "overlays"
    for role in roles_in(lock.get("taken", [])):
        stub = overlays / ("%s.md" % role)
        if not stub.exists() or not stub.read_text(encoding="utf-8").strip():
            failures.append("overlay stub missing or empty: overlays/%s.md" % role)

    if failures:
        print("validate: FAIL")
        for failure in failures:
            print("  - %s" % failure)
        return 1
    print("validate: OK (%s core files, %s forks, renders current, provenance %s)"
          % (len(lock.get("files", {})), len(lock.get("forks", {})),
             lock.get("provenance_mode")))
    print("operator read check (from the framework checkout, not part of validate):")
    print("  agentic status --repo %s" % target)
    return 0


def cmd_fork(args):
    target = Path(args.target).resolve()
    lock_path = find_lock(target, args.prefix)
    lock = load_json(lock_path)
    host_rel = args.file
    if host_rel not in lock["files"]:
        raise SystemExit("%s is not a taken core-layer file in the lock" % host_rel)
    if host_rel in lock["forks"]:
        raise SystemExit("%s is already recorded as a fork" % host_rel)
    path = target / host_rel
    if sha256(path) != lock["files"][host_rel]:
        raise SystemExit(
            "%s has already been edited; restore the pristine copy first, then "
            "fork, then re-apply your edit" % host_rel
        )
    base = target / lock["prefix"] / "upstream" / host_rel
    base.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(str(path), str(base))
    lock["forks"][host_rel] = {
        "base_sha256": lock["files"][host_rel],
        "reason": args.reason,
        "review_at_upgrade": True,
    }
    lock_path.write_text(json.dumps(lock, indent=2) + "\n", encoding="utf-8")
    print("forked %s (pristine base retained at %s); it is now yours to edit"
          % (host_rel, base.relative_to(target).as_posix()))
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    sub = parser.add_subparsers(dest="command", required=True)

    p_init = sub.add_parser("init", help="scaffold the framework into a host repo")
    p_init.add_argument("target")
    p_init.add_argument("--take", default="all",
                        help="all | sdlc | comma-separated file list")
    p_init.add_argument("--layout", choices=["prefixed", "root"], default="prefixed")
    p_init.add_argument("--prefix", default=".agentic")
    p_init.add_argument("--provenance", choices=["redistribute", "private"],
                        required=True,
                        help="the host's posture; no default on purpose")
    p_init.add_argument("--adapters", default="auto")
    p_init.set_defaults(func=cmd_init)

    p_val = sub.add_parser("validate", help="static integration checks")
    p_val.add_argument("target", nargs="?", default=".")
    p_val.add_argument("--prefix", default=".agentic")
    p_val.set_defaults(func=cmd_validate)

    p_fork = sub.add_parser("fork", help="record a deliberate core-file divergence")
    p_fork.add_argument("file", help="host-relative path of the taken core file")
    p_fork.add_argument("--reason", required=True)
    p_fork.add_argument("--target", default=".")
    p_fork.add_argument("--prefix", default=".agentic")
    p_fork.set_defaults(func=cmd_fork)

    args = parser.parse_args()
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
