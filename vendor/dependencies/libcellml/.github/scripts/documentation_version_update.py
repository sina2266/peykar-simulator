"""
Update the website version information.

Creates a JSON file with content on available documentation for
version.  The JSON file follows the correct specification for
the libCellML website to understand and use.
"""
import argparse
import json
import os
import shutil

from packaging.version import InvalidVersion, Version

from utilities import unwanted_versions


def do_documentation_version_update(website_src_dir):
    documentation_versions = os.listdir(f"{website_src_dir}/public/generated/")
    only_documentation_versions = []
    for possible in documentation_versions:
        try:
            acceptable = Version(possible)
            only_documentation_versions.append(possible)
        except InvalidVersion:
            pass
    only_documentation_versions.sort(key=Version)
    only_documentation_versions.reverse()
    _cleanup_versions(website_src_dir, only_documentation_versions)

    _write_versions_file(website_src_dir, only_documentation_versions)

    for version in only_documentation_versions:
        _write_documentation_availability_file(f"{website_src_dir}/public/generated/" + version)


def _cleanup_versions(website_src_dir, versions):
    remove_versions = unwanted_versions(versions)

    for version in remove_versions:
        shutil.rmtree(f"{website_src_dir}/public/generated/{version}")


def _write_versions_file(website_src_dir, versions):
    content = f"""// This file is generated do not edit!
// Changes made here will be overwritten when a release is made.
export const getDocumentationVersions = () => {{
  return {versions}
}}
"""

    with open(f"{website_src_dir}/src/js/documentationversions.js", "w") as f:
        f.write(content)


def _write_documentation_availability_file(location):
    directories = next(os.walk(location))[1]
    with open(os.path.join(location, "directories.json"), "w") as f:
        json.dump(directories, f)


def _process_arguments():
    parser = argparse.ArgumentParser(
        description="Update the documentation version information of the libCellML website.")
    parser.add_argument("directory",
                        help="Relative directory from current working directory"
                             " to directory to libCellML website source.")

    return parser.parse_args()


def main():
    args = _process_arguments()
    do_documentation_version_update(args.directory)


if __name__ == "__main__":
    main()

