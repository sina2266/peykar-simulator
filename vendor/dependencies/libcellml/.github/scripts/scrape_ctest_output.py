import re
import json
import sys
from collections import defaultdict


def extract_failures(text):
    """
    Extract failed test names from CTest output.
    """
    failures = []

    capture = False
    for line in text.splitlines():
        line = line.strip()

        # Start capturing after this marker
        if line.startswith("The following tests FAILED"):
            capture = True
            continue

        if capture:
            # Stop on empty line or non-matching content
            if not line or not re.match(r"\d+\s+-\s+", line):
                break

            # Extract test name
            match = re.match(r"\d+\s+-\s+(.+?)\s+\(Failed\)", line)
            if match:
                failures.append(match.group(1))

    return failures


def extract_test_declarations(text):
    capture = False
    declarations = []
    for line in text.splitlines():
        line = line.strip()
        if line.startswith("Test project"):
            capture = True
            continue
        if line.startswith("48% tests passed"):
            capture = False
            continue

        if capture:
            match = re.match(r"Start\s+\d+:\s(.+?)$", line)
            if match:
                declarations.append(match.group(1))

    return declarations


def normalise_group(test_name):
    """
    Convert test name to a grouping key.
    """
    # Remove leading "py" prefix
    if test_name.startswith("py"):
        test_name = test_name[2:]

    # Take first segment before '/'
    group = test_name.split("/")[0]

    return group


def count_failures_by_group(failures):
    """
    Count failures per group.
    """
    counts = defaultdict(int)

    for test in failures:
        group = normalise_group(test)
        counts[group] += 1

    return dict(counts)


def count_test_groups(declarations):
    """
    Count the number of tests per group.
    """
    counts = defaultdict(int)
    for group in declarations:
        group = normalise_group(group)
        counts[group] += 1
        
    return dict(counts)


def main():
    # Read full input (pipe or file)
    text = sys.stdin.read()

    failures = extract_failures(text)
    declarations = extract_test_declarations(text)
    counts = count_failures_by_group(failures)
    groups = count_test_groups(declarations)

    result = {
        "total_failed": len(failures),
    }
    for group in groups:
        fails = counts.get(group, 0)
        result[group] = { "passed": groups[group] - fails, "failed": fails }

    print(json.dumps(result, indent=2))


if __name__ == "__main__":
    main()
