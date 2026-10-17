import argparse
import os
import sys

from packaging.version import Version

from abi.buildbotscripts.libcellml.utilities import unwanted_versions

CHANGELOG_PREFIX = "changelog_v"
CHANGELOG_TOC_START_MARKER = "  # CHANGELOG_TOC_START_MARKER\n"
CHANGELOG_TOC_END_MARKER = "  # CHANGELOG_TOC_END_MARKER\n"


def _get_changelog_file_names(location):
    current_dir = os.path.abspath(os.path.curdir)
    working_dir = os.path.join(current_dir, location)
    os.chdir(working_dir)

    changelogs = []
    for root, dirs, files in os.walk(".", topdown=False):
        for name in files:
            parts = os.path.splitext(name)
            if len(parts) == 2 and parts[0].startswith(CHANGELOG_PREFIX) and parts[1] == ".rst":
                # changelogs.append(os.path.join(root, name))
                changelogs.append(parts[0].replace(CHANGELOG_PREFIX, ''))

    os.chdir(current_dir)

    return changelogs


def _remove_changelogs(location, versions):
    for version_ in versions:
        os.remove(os.path.join(location, f"{CHANGELOG_PREFIX}{version_}.rst"))


def _create_text(versions, with_directory=False):
    if with_directory:
        changelog_dir = "changelogs/"
    else:
        changelog_dir = ""

    text = [
        "\n",
        "Changelogs\n",
        "==========\n",
        "\n",
        ".. toctree::\n",
        "\n",
    ]
    for v in versions:
        text.append(f"   {changelog_dir}{CHANGELOG_PREFIX}{v}\n")
    text.append("\n")

    return text


def _write_changelog_index_file(file_name, versions):
    text = _create_text(versions)

    with open(file_name, 'w') as f:
        for line in text:
            f.write(line)


def _write_main_index_file(file_name, versions):
    with open(file_name) as f:
        lines = f.readlines()

    start_index = lines.index(CHANGELOG_TOC_START_MARKER)
    # We subtract one line to include the restructured text comment marker.
    end_index = lines.index(CHANGELOG_TOC_END_MARKER) - 1
    target_lines = [j for i, j in enumerate(lines)
                    if not start_index < i < end_index]

    text = _create_text(versions, with_directory=True)

    # Insert our generated changelog toc into current file content.
    insert_index = start_index + 1
    target_lines[insert_index:insert_index] = text

    with open(file_name, 'w') as f:
        for line in target_lines:
            f.write(line)


def process_arguments():
    parser = argparse.ArgumentParser(description="Update the table of contents to match existing changelogs.")
    parser.add_argument("local_repo",
                        help="The location of the project repository. Absolute path or relative path relative to the "
                             "current working directory.")

    return parser


def main():
    parser = process_arguments()
    args = parser.parse_args()
    repo_relative_path = args.local_repo

    cur_dir = os.path.abspath(os.curdir)

    repo_path = os.path.join(cur_dir, repo_relative_path)
    if not os.path.isfile(os.path.join(repo_path, 'CMakeLists.txt')):
        sys.exit(2)

    if not os.path.isdir(os.path.join(repo_path, 'docs', 'changelogs')):
        sys.exit(3)

    changelogs = _get_changelog_file_names(os.path.join(repo_path, 'docs', 'changelogs'))
    sorted_changelogs = sorted(changelogs, key=Version)
    sorted_changelogs.reverse()

    remove_versions = unwanted_versions(sorted_changelogs, depth=3)
    wanted_changelogs = list(filter(lambda x: x not in remove_versions, sorted_changelogs))
    _remove_changelogs(os.path.join(repo_path, 'docs', 'changelogs'), remove_versions)

    main_index_file = os.path.join(repo_path, 'docs', 'index.rst')
    _write_main_index_file(main_index_file, wanted_changelogs)

    changelog_index_file = os.path.join(repo_path, 'docs', 'changelogs', 'index.rst')
    _write_changelog_index_file(changelog_index_file, wanted_changelogs)


if __name__ == "__main__":
    main()
