"""Explicit operator replay of checksum-pinned original plotting code, headless."""

import argparse
import hashlib
import importlib.util
import json
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import pandas as pd

PLOT_SHA = "8d132b8c9335d47e4e600b9101704d6351cd0f5fac3d0a56b392a0187ed94838"
MEMBRANE_SHA = "742c10584f052f2955fbb6f543b7456ab33b1e0267a5bf0c7abd16137b20e6b0"


GROUPS = {
    "self-organization": ("STEM_ONLY_proper_thickness_6months/Comb0", 21),
    "slight-injury": ("(Slight) 750_G/Comb0", 13),
    "mild-injury": ("(Mild) 1500_G/Comb1", 16),
    "moderate-injury": ("(Moderate) 2500_G/Comb1", 15),
}


def run(source: Path, data: Path, output: Path, *, groups=None) -> None:
    if output.exists():
        raise ValueError("Choose a new analysis output; previous results are immutable")
    for name, checksum in (("plot_manager.py", PLOT_SHA), ("membrane_height.csv", MEMBRANE_SHA)):
        if hashlib.sha256((source / name).read_bytes()).hexdigest() != checksum:
            raise ValueError("Original analysis source checksum differs")
    output.mkdir(parents=True)
    module_spec = importlib.util.spec_from_file_location(
        "original_cornea_plot_manager", source / "plot_manager.py"
    )
    module = importlib.util.module_from_spec(module_spec)
    module_spec.loader.exec_module(module)
    groups = GROUPS if groups is None else groups
    receipt = {
        "original_plot_digest": PLOT_SHA,
        "original_membrane_digest": MEMBRANE_SHA,
        "analysis_wrapper_digest": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        "figure_export": "Export layout/bounds; original titles, labels and math retained",
        "groups": {},
        "input_files": {},
        "scope": (
            "Unmodified archived plotting mathematics; show saves PNG/PDF. "
            "Archive count plots truncate to4320h; full injury raw horizon5040h is retained."
        ),
    }
    for label, (relative, expected) in groups.items():
        folder = output / label
        folder.mkdir()
        manager = module.PlotManager(
            Data_DIR=data / relative,
            Stable_time=500,
            Max_time=4320,
            SimTime_mcs=4320,
            Out_DIR=folder,
        )
        if len(manager.count_data) != expected or len(manager.thickness_data) != expected:
            raise ValueError(f"Missing/colliding original cohort rows for {label}")
        if manager.memb_bins is None or set(manager.memb_bins) != set(range(10)):
            raise ValueError("Original membrane bin mapping is unavailable")
        for name in manager.Count_file + manager.Thickness_file:
            path = Path(name)
            receipt["input_files"][str(path.relative_to(data))] = hashlib.sha256(
                path.read_bytes()
            ).hexdigest()
        figures = []

        def save_current(figures=figures, folder=folder):
            number = len(figures) + 1
            figure = plt.gcf()
            figure.tight_layout(pad=1.4)
            figure.canvas.draw()
            for suffix in ("png", "pdf"):
                path = folder / f"original-{number}.{suffix}"
                figure.savefig(path, dpi=160, bbox_inches="tight", pad_inches=0.15)
                figures.append(str(path.relative_to(output))) if suffix == "png" else None
            plt.close("all")

        plt.show = save_current
        manager.plot_count()
        if label == "self-organization":
            manager.plot_thickness()
        count_data = pd.concat(manager.count_data.values())
        mean = count_data.groupby("Time").mean(numeric_only=True).reset_index()
        mean.to_csv(folder / "full-horizon-count-mean.csv", index=False)
        summaries = []
        for replicate, frame in manager.count_data.items():
            stable = frame[frame.Time >= 500]
            summaries.append(
                {
                    "replicate": replicate,
                    "samples": len(frame),
                    "end_hour": float(frame.Time.iloc[-1]),
                    "stable_cell_means": {
                        name: float(stable[name].mean())
                        for name in ["Superficial", "Wing", "Basal", "Stem"]
                    },
                }
            )
        receipt["groups"][label] = {
            "replicates": expected,
            "replicate_summaries": summaries,
            "figures": figures,
            "original_membrane_bins": manager.memb_bins,
        }
        print(label, "original analysis passed", expected, "replicates", flush=True)
    receipt["files"] = {
        str(p.relative_to(output)): hashlib.sha256(p.read_bytes()).hexdigest()
        for p in output.rglob("*")
        if p.is_file()
    }
    (output / "receipt.json").write_text(json.dumps(receipt, indent=2) + "\n")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", required=True, type=Path)
    parser.add_argument("--data", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    run(args.source, args.data, args.output)
