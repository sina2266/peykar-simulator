# Declarative native model fixtures

`transfer/1.0.0` contains an authored synthetic SBML A→B reaction with analytical
reference `A(t)=exp(-0.1 t)`, `A+B=1` for seconds and a one-litre compartment.
Its source text uses the Peykar project license and proves numerical execution only.

`aciclovir/1.0.0` contains admission/plan metadata for the original OSPSuite-R PKML
at `vendor/dependencies/ospsuite-r/inst/extdata/Aciclovir.pkml`. `source.json` pins
that exact existing file; no derivative model or altered schedule is substituted.
The original Open Systems Pharmacology authors and GPL-2 plus clarifying addendum
remain in that vendored project's `DESCRIPTION`/`LICENSE` and root third-party
notices. The model remains under its original terms, separately from Peykar.

```sh
uv run peykar native register-model model_packages/native/aciclovir/1.0.0/spec.json \
  --model vendor/dependencies/ospsuite-r/inst/extdata/Aciclovir.pkml
uv run peykar native submit model_packages/native/aciclovir/1.0.0/plan.json
```

These require the reviewed OSP execution profile to be admitted in the same local
state and a separate native agent to execute the job. For Compose, mount the
metadata/model read-only into an operator container sharing the existing volume;
host `.peykar` is a different registry. Native execution is not independent
biological validation. See [native operations](../../docs/native-agent.md).
