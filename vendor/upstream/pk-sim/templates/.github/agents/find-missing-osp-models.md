---
name: find-missing-models
description: "Find public OSP model repositories in the Open-Systems-Pharmacology GitHub organization that are not yet referenced in templates.json and have at least one release."
---

# Find Missing OSP Model Repositories

You are an agent that identifies public repositories in the **Open-Systems-Pharmacology** GitHub organization which have the `osp-model` topic, have at least one release, and are **not** yet referenced in the `templates.json` file of this repository.

## Step 1 — Gather Data

1. **Search repositories**: Use the GitHub search API (or MCP tools) to find all public repositories in the `Open-Systems-Pharmacology` organization that have the topic **`osp-model`**. Paginate through all results — the organization may have 200+ matching repositories.

2. **Parse `templates.json`**: Read the file `templates.json` from this repository and extract all unique repository names from every `Url` field. Each URL follows the pattern:
   ```
   https://raw.githubusercontent.com/Open-Systems-Pharmacology/<REPO_NAME>/<version>/<file>
   ```
   Multiple templates may reference the same repository — deduplicate the extracted names.

## Step 2 — Apply Filters

From the full list of `osp-model` repositories, **exclude** any repository matching **any** of these conditions:

| # | Condition |
|---|-----------|
| 1 | Repository name is exactly one of: `Glucose-Insulin-Model`, `Pregnancy-Models`, `Thyroid-Hormones-PB-QSP-Model` |
| 2 | Repository name starts **or** ends with `Example` (case-insensitive) |
| 3 | Repository name starts **or** ends with `DDI`, `DDGI`, or `Pediatrics` (case-insensitive) |
| 4 | Repository name already appears in any `Url` node of `templates.json` |
| 5 | Repository has **zero** releases (including pre-releases) — only keep repos with **at least one** release |

## Step 3 — Output

Print the final list of matching repository URLs, one per line:

```
https://github.com/Open-Systems-Pharmacology/<REPO_NAME>
```

## Important Notes

- When checking releases, include pre-releases — a repo with only pre-releases still counts as having a release.
- The GitHub search API returns at most 100 results per page; always paginate through all pages.
- All repositories returned by the topic search in this org are public.
