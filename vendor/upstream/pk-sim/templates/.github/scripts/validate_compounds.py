import json
import requests
import sys
import os

def main():
    # Find the path to templates.json relative to this script
    repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../"))
    templates_json_path = os.path.join(repo_root, "templates.json")

    # Parse compound names to ignore from command line argument, if given
    ignore_arg = sys.argv[1] if len(sys.argv) > 1 else ""
    ignore_names = set([name.strip() for name in ignore_arg.split("|") if name.strip()])

    # Load templates.json
    with open(templates_json_path, "r", encoding="utf-8") as f:
        data = json.load(f)

    # Get all compound entries and their URLs
    compound_templates = [
        t for t in data.get("Templates", [])
        if t.get("Type") == "Compound"
    ]
    template_names = set(t.get("Name") for t in compound_templates)

    missing = []

    for template in compound_templates:
        url = template.get("Url")
        try:
            resp = requests.get(url, timeout=30)
            resp.raise_for_status()
            compound_json = resp.json()
        except Exception as e:
            print(f"Error retrieving/parsing {url}: {e}", file=sys.stderr)
            sys.exit(1)
        # Extract compound names from referenced json
        compounds = compound_json.get("Compounds", [])
        for compound in compounds:
            compound_name = compound.get("Name")
            if compound_name in ignore_names:
                continue  # Skip ignored compounds
            if compound_name not in template_names:
                missing.append({
                    "compound_name": compound_name,
                    "referenced_snapshot": url
                })

    if missing:
        print("Missing compound template entries for the following combinations:\n")
        for miss in missing:
            print(f"- Compound: '{miss['compound_name']}', Referenced snapshot: {miss['referenced_snapshot']}")
        sys.exit(1)
    else:
        print("All referenced compound names are present in templates.json.")

if __name__ == "__main__":
    main()