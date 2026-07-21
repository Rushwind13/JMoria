#!/usr/bin/env python3
"""Render adapter agent files from the runtime-neutral role specs.

Source of truth: roles/<role>.md (frontmatter + body).
Per-adapter mapping: adapters/<adapter>/manifest.json.
Output: the agent files each runner loads (.claude/agents/, .github/agents/, ...).

Usage:
    python3 scripts/render-agents.py            # (re)write all rendered files
    python3 scripts/render-agents.py --check    # exit 1 if any rendered file is stale

Stdlib-only and Python 3.9-compatible on purpose (see plan ADR-8 of the wordfreq
run): JSON manifests instead of YAML, no PEP 604 annotations.
"""
import json
import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
HEADER = "<!-- RENDERED from roles/{role}.md by scripts/render-agents.py - DO NOT EDIT.\n     Edit the role spec, then run: python3 scripts/render-agents.py -->"


def overlay_for(role):
    """Project policy layer (docs/INTEGRATION.md §4): overlays/_all.md is
    spliced into every agent, overlays/<role>.md into that role's agent.
    A stub that is only HTML comments splices nothing."""
    parts = []
    for name in ("_all", role):
        path = REPO / "overlays" / ("%s.md" % name)
        if not path.exists():
            continue
        text = path.read_text(encoding="utf-8")
        if not re.sub(r"<!--.*?-->", "", text, flags=re.S).strip():
            continue
        parts.append(
            "<!-- OVERLAY from overlays/%s.md - project policy layer -->\n\n%s"
            % (name, text.strip())
        )
    return parts


def parse_role(path):
    """Split a role spec into (frontmatter dict, body). Flat keys only."""
    text = path.read_text(encoding="utf-8")
    parts = text.split("---\n")
    if len(parts) < 3 or parts[0].strip():
        raise SystemExit("%s: expected leading '---' frontmatter block" % path)
    front = {}
    for line in parts[1].splitlines():
        if not line.strip() or line.lstrip() != line:  # skip blanks/nested
            continue
        key, _, value = line.partition(":")
        front[key.strip()] = value.strip()
    body = "---\n".join(parts[2:]).lstrip("\n")
    return front, body


def parse_list(value):
    """Parse a '[a, b, c]' frontmatter value into a list."""
    return [item.strip() for item in value.strip("[]").split(",") if item.strip()]


def merge_tools(capabilities, manifest):
    tools = []
    for cap in capabilities:
        if cap not in manifest["tool_map"]:
            raise SystemExit(
                "%s: capability '%s' missing from tool_map" % (manifest["adapter"], cap)
            )
        for tool in manifest["tool_map"][cap]:
            if tool not in tools:
                tools.append(tool)
    return tools


def render_tools(capabilities, manifest):
    tools = merge_tools(capabilities, manifest)
    if manifest["tools_style"] == "comma":
        return ", ".join(tools)
    return "[" + ", ".join(tools) + "]"


def render_permission_map(capabilities, manifest):
    """opencode-style scoping: agents carry a permission map (allow/deny), not a
    tool list. Deny-by-default plus explicit allows keeps the adapter rule
    (narrow, never widen) a rendered property, and new harness tools stay denied
    until a capability grants them."""
    lines = ["permission:", '  "*": deny']
    for key in merge_tools(capabilities, manifest):
        lines.append("  %s: allow" % key)
    return lines


def render_agent(role, manifest):
    front, body = parse_role(REPO / "roles" / ("%s.md" % role))
    for required in ("dispatch", "capabilities", "capability_profile"):
        if required not in front:
            raise SystemExit("roles/%s.md: missing '%s' in frontmatter" % (role, required))
    model = manifest["model_overrides"].get(
        role, manifest["model_map"].get(front["capability_profile"])
    )
    if model is None:
        raise SystemExit(
            "%s: no model for profile '%s' (role %s)"
            % (manifest["adapter"], front["capability_profile"], role)
        )
    lines = [
        "---",
        "name: %s" % role,
        "description: %s" % front["dispatch"],
    ]
    capabilities = parse_list(front["capabilities"])
    if manifest["tools_style"] == "permission-map":
        lines += render_permission_map(capabilities, manifest)
    else:
        lines.append("tools: %s" % render_tools(capabilities, manifest))
    lines.append("model: %s" % model)
    for key, value in manifest.get("extra_frontmatter", {}).items():
        lines.append("%s: %s" % (key, json.dumps(value)))
    lines += ["---", "", HEADER.format(role=role), "", body.rstrip()]
    for overlay in overlay_for(role):
        lines += ["", overlay.rstrip()]
    return "\n".join(lines) + "\n"


def main():
    check = "--check" in sys.argv[1:]
    stale = []
    for manifest_path in sorted(REPO.glob("adapters/*/manifest.json")):
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
        out_dir = REPO / manifest["output_dir"]
        out_dir.mkdir(parents=True, exist_ok=True)
        for role in manifest["roles"]:
            rendered = render_agent(role, manifest)
            out_path = out_dir / manifest["filename"].format(role=role)
            current = out_path.read_text(encoding="utf-8") if out_path.exists() else None
            if current != rendered:
                if check:
                    stale.append(str(out_path.relative_to(REPO)))
                else:
                    out_path.write_text(rendered, encoding="utf-8")
                    print("rendered %s" % out_path.relative_to(REPO))
    if check:
        if stale:
            print("STALE (run: python3 scripts/render-agents.py):")
            for path in stale:
                print("  %s" % path)
            return 1
        print("all rendered agents up to date")
    return 0


if __name__ == "__main__":
    sys.exit(main())
