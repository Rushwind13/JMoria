# Framework metadata

This directory pins this repository's integration of the Agentic
Development System (source: git@github.com:nathancrtr/agentic-sandbox.git, ref 1aef44ebb4cd). `framework-lock.json` is the
authoritative record of what is framework core versus instance-local;
`upstream/` retains the pristine base of any recorded fork. Do not edit
core-layer copies in place — extend via `overlays/` or record a fork:
`python3 scripts/integrate.py fork <file> --reason "..."`.
