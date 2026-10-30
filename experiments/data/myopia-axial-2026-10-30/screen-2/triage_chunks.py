"""Masked triage chunks for screen-2 tranche 1 (structure only, no values).

Run from the repository root with ``uv run --frozen python
experiments/data/myopia-axial-2026-10-30/screen-2/triage_chunks.py --tranche 1
--output <new directory>``.

The first screen's ``triage_chunks.py`` (its ``block`` formatting, unchanged) with this
screen's tranche order and masked previews: articles of the tranche are taken in rank
order and dealt round-robin into ``--chunks`` files; each article shows its masked
qualifying tables (data cells and numbers in labels hidden by
``show_masked_previews.strict``).
"""

import argparse
import importlib.util
import json
import sys
from pathlib import Path
from types import ModuleType

HERE = Path(__file__).resolve().parent


def _load(name: str, path: Path) -> ModuleType:
    spec = importlib.util.spec_from_file_location(name, path)
    if spec is None or spec.loader is None:
        raise SystemExit(f"cannot load {path}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[name] = module
    spec.loader.exec_module(module)
    return module


# The first screen's chunk formatting (it imports the unchanged rule and masking).
FIRST = _load("myopia_screen1_triage_chunks", HERE.parent / "screening/triage_chunks.py")
block = FIRST.block
PREVIEWS = _load("myopia_screen2_tranche_order", HERE / "tranche_order.py").PREVIEWS
ORDER = HERE / "tranche-order-1.0.0.json"


def chunks(tranche: int, count: int) -> list[tuple[list[str], str]]:
    order = json.loads(ORDER.read_text())
    ids = [
        a["pmcid"]
        for a in sorted(order["articles"], key=lambda a: a["rank"])
        if a["tranche"] == tranche
    ]
    out = []
    for k in range(count):
        members = ids[k::count]
        text = "\n".join(
            block(json.loads((PREVIEWS / f"{pmcid}.json").read_text())) for pmcid in members
        )
        out.append((members, text + "\n"))
    return out


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--tranche", type=int, required=True)
    parser.add_argument("--chunks", type=int, default=3)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists():
        raise SystemExit("--output must be a new directory")
    args.output.mkdir(parents=True)
    for k, (members, text) in enumerate(chunks(args.tranche, args.chunks), start=1):
        (args.output / f"chunk-{k}-ids.json").write_text(json.dumps(members) + "\n")
        (args.output / f"chunk-{k}.txt").write_text(text)
        print(k, len(members), len(text))


if __name__ == "__main__":
    main()
