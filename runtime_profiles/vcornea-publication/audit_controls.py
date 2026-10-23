"""Read pinned parameter definitions and original DOCX tables without importing model code."""

import argparse
import ast
import hashlib
import json
import xml.etree.ElementTree as ET
import zipfile
from pathlib import Path

NS = {"w": "http://schemas.openxmlformats.org/wordprocessingml/2006/main"}


def literal(node):
    # The reviewed HPC dictionary uses 7200*7 for its horizon. No code evaluation.
    if isinstance(node, ast.BinOp) and isinstance(node.op, ast.Mult):
        left, right = literal(node.left), literal(node.right)
        if type(left) not in (int, float) or type(right) not in (int, float):
            raise ValueError("Only numeric literal multiplication is supported")
        return left * right
    if isinstance(node, ast.List):
        return [literal(value) for value in node.elts]
    if isinstance(node, ast.Dict):
        return {literal(k): literal(v) for k, v in zip(node.keys, node.values, strict=True)}
    return ast.literal_eval(node)


def source_values(path, *, hpc=False):
    values = {}
    for node in ast.parse(path.read_bytes()).body:
        if not isinstance(node, ast.Assign):
            continue
        for target in node.targets:
            if not isinstance(target, ast.Name):
                continue
            if hpc and target.id == "var_dict":
                return literal(node.value)
            if not hpc:
                try:
                    values[target.id] = literal(node.value)
                except (ValueError, TypeError, SyntaxError):
                    continue
    if hpc:
        raise ValueError("Reviewed HPC parameter dictionary is missing")
    return values


def same_value(left, right):
    if isinstance(left, bool) or isinstance(right, bool):
        return type(left) is type(right) and left == right
    return left == right


def audit(source, pins, supplements, catalog, output):
    if output.exists():
        raise ValueError("Preserve previous audit output")
    source_pins = json.loads(pins.read_bytes())["files"]
    originals = json.loads(catalog.read_bytes())["paper_supplements"]
    definitions = {}
    evidence = {}
    for name, relative, hpc in [
        (
            "local_formation",
            "Local/Project/paper_version_STEM_ONLY/Simulation/Parameters.py",
            False,
        ),
        ("local_injury", "Local/Project/paper_version/Simulation/Parameters.py", False),
        ("hpc_formation", "HPC/Batch_Run_vCornea_Paper_version_STEM_ONLY_noVTK.py", True),
        ("hpc_injury", "HPC/Batch_Run_vCornea_Paper_version.py", True),
    ]:
        path = source / relative
        sha = hashlib.sha256(path.read_bytes()).hexdigest()
        if sha != source_pins[relative]:
            raise ValueError("Original parameter source identity differs")
        evidence[relative] = sha
        definitions[name] = source_values(path, hpc=hpc)
    rows = []
    for supplement in originals:
        if supplement["id"] not in ("s013", "s014", "s015", "s016"):
            continue
        path = supplements / f"{supplement['id']}.docx"
        sha = hashlib.sha256(path.read_bytes()).hexdigest()
        if sha != supplement["sha256"]:
            raise ValueError("Original final-paper table identity differs")
        evidence[path.name] = sha
        with zipfile.ZipFile(path) as archive:
            doc = ET.fromstring(archive.read("word/document.xml"))
        for tr in doc.findall(".//w:tr", NS):
            cells = [
                "".join(t.text or "" for t in tc.findall(".//w:t", NS))
                for tc in tr.findall("w:tc", NS)
            ]
            if len(cells) < 3 or cells[0].strip() not in definitions["local_injury"]:
                continue
            name, paper = cells[0].strip(), cells[2].strip()
            row = {
                "parameter": name,
                "supplement": supplement["id"],
                "paper_text": paper,
                "source_values": {key: values.get(name) for key, values in definitions.items()},
            }
            try:
                value = ast.literal_eval(paper.replace(",", ""))
                row["paper_literal"] = value
                row["local_injury_agrees"] = same_value(value, definitions["local_injury"][name])
            except (ValueError, SyntaxError):
                row["local_injury_agrees"] = None
            rows.append(row)
    result = {
        "schema_version": "1.0.0",
        "class": "source_correspondence_audit",
        "source_files": evidence,
        "table_parameters": rows,
        "limitations": [
            "Batch defaults are published; seeds/per-run parameter snapshots were not archived.",
            "Table S11 describes formation/debug flags and mislabels InjuryType semantics.",
            "Source False/0 selects chemical injury; True/1 selects geometric ablation.",
            "A table Boolean SLS_Threshold is not substituted for the numeric source threshold2.0.",
            "Formation batch40/8/12 and final tables25/7/20 are distinct configurations.",
        ],
    }
    output.write_text(json.dumps(result, indent=2, allow_nan=False) + "\n")
    print(len(rows), "named table entries audited; unresolved source/table conflicts retained")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", required=True, type=Path)
    parser.add_argument("--pins", required=True, type=Path)
    parser.add_argument("--supplements", required=True, type=Path)
    parser.add_argument("--catalog", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    audit(args.source, args.pins, args.supplements, args.catalog, args.output)
