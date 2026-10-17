import argparse
import re
import requests

EXTERNAL_REFERENCE_REGEX = re.compile(r"set\(TEST_EXTERNAL_REFERENCE (.*)\)")


def _get_remote_info(url, regex):
    non_quoted_url = url.replace('"', "")
    non_quoted_url = non_quoted_url.replace("'", "")
    response = requests.get(non_quoted_url)

    return _extract_info(response.text.split("\n"), regex)


def _extract_info(content, regex):

    # set(TEST_GIT_REPO "https://github.com/libcellml/userguides")
    # set(TEST_COMMIT_HASH "v0.1.0")
    for line in content:
        m1 = regex.match(line)
        if m1:
            return m1.group(1)

        m2 = EXTERNAL_REFERENCE_REGEX.match(line)
        if m2:
            return _get_remote_info(m2.group(1), regex)

    return "hogwash"


def get_repo_info(location):
    regex = re.compile(r"set\(TEST_GIT_REPO (.*)\)")
    with open(location) as f:
        content = f.readlines()

    return _extract_info(content, regex)


def get_branch_info(location):
    regex = re.compile(r"set\(TEST_COMMIT_HASH (.*)\)")
    with open(location) as f:
        content = f.readlines()

    return _extract_info(content, regex)


def _process_arguments():
    parser = argparse.ArgumentParser(
        description="Determine the information for the requested aspect of the "
                    "libcellml_tutorials.cmake file from the End-to End database.")
    parser.add_argument("aspect", help="Either 'repo' or 'branch'.")
    parser.add_argument("location",
                        help="Location of the test suite definition file.")

    return parser.parse_args()


def main():
    args = _process_arguments()
    if args.aspect == 'repo':
        info = get_repo_info(args.location)
    else:
        info = get_branch_info(args.location)

    print(info.replace('"', ''))


if __name__ == "__main__":
    main()
