import json
import requests
import sys
import os

TEMPLATE_FILE = "templates.json"

def pluralize(type_str):
    # Very naive pluralization, works for nouns ending in consonant or 'y'
    # Can be extended if needed
    if type_str.endswith('y') and not type_str.endswith('ay') and not type_str.endswith('ey') and not type_str.endswith('iy') and not type_str.endswith('oy') and not type_str.endswith('uy'):
        return type_str[:-1] + 'ies'
    elif type_str.endswith('s'):
        return type_str + 'es'
    else:
        return type_str + 's'

def main():
    if not os.path.exists(TEMPLATE_FILE):
        print(f"File '{TEMPLATE_FILE}' not found.")
        sys.exit(1)

    with open(TEMPLATE_FILE, encoding="utf-8") as f:
        data = json.load(f)

    errors = []
    for idx, entry in enumerate(data.get("Templates", [])):
        type_name = entry.get("Type")
        template_name = entry.get("Name")
        url = entry.get("Url")
        if not (type_name and template_name and url):
            errors.append({
                "index": idx,
                "reason": "Missing Type, Name, or Url",
                "object": entry
            })
            continue

        node_name = pluralize(type_name)
        try:
            response = requests.get(url, timeout=30)
            response.raise_for_status()
            ref_json = response.json()
        except Exception as e:
            errors.append({
                "index": idx,
                "reason": f"Failed to fetch or parse referenced file: {e}",
                "object": entry
            })
            continue

        if node_name not in ref_json or not isinstance(ref_json[node_name], list):
            errors.append({
                "index": idx,
                "reason": f"Referenced file does not contain array node '{node_name}'",
                "object": entry
            })
            continue

        found = any(obj.get("Name") == template_name for obj in ref_json[node_name] if isinstance(obj, dict))
        if not found:
            errors.append({
                "index": idx,
                "reason": f"No object in '{node_name}' with Name='{template_name}'",
                "object": entry
            })

    if errors:
        print("Validation failed for the following templates:")
        for err in errors:
            print(f"\nIndex: {err['index']}\nReason: {err['reason']}\nObject: {json.dumps(err['object'], ensure_ascii=False, indent=2)}")
        sys.exit(1)
    else:
        print("All templates validated successfully.")

if __name__ == "__main__":
    main()