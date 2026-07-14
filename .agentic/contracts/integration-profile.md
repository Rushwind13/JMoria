# Integration Profile: <host repo>

<!-- Contract: produced by Integrator; consumed by the GI human and by every
     later run in this host. Gate: GI (the scaffold PR review — "is this how
     agents should behave in this house?"). All sections required; a guardrail
     that traces to no probe finding or host policy is malformed.
     BUDGET: reviewable by the GI human in fifteen minutes. -->

## Environment probe
<!-- Interpreters and versions, package-tooling traps, hook health on pristine
     main, what can and cannot run locally. Commands run, output pasted. -->

## Gate mapping
<!-- The host's machine-enforced gates (CI, SAST, branch protection,
     deploy-on-merge) mapped onto the gate set this host adopts: G0–G3 for
     SDLC hosts, the host's own gate map otherwise. Include any deploy weight
     a merge already carries. -->

## Conventions map
<!-- Where the host's conventions live and whether cold-start agents in fresh
     clones can read them. A gitignored source is a finding with a remediation
     proposal. -->

## Guardrail register
<!-- Host-specific hard rules (infra mutation, secrets and sensitive data in
     artifacts, header policy), each traced to a probe finding or cited host
     policy. -->
| # | Guardrail | Traces to |
|---|-----------|-----------|

## Decorrelation assessment
<!-- Which vendors are reachable in this org; whether P5 is satisfiable or is
     recorded as a known weakening, and for which role pairs. -->

## Runs location
<!-- In-repo runs/ vs. sidecar repo, with the host's compliance posture
     stated as the decision input. -->

## Dispatch reality
<!-- Which operating modes this host actually runs: interactive operator
     sessions, scheduled/self-dispatching runs (gate entries stay human-only
     regardless), or the autonomous orchestrator. -->

## Gate GI record
<!-- Written ONLY by the named human approver, like any gate. -->
- approved: <true|false>
- by: <name>
- at: <date>
- notes: <what was vetoed or amended>
