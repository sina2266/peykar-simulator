import argparse
import os
import subprocess

here = os.path.abspath(os.path.dirname(__file__))


def run_set_version(location, core, developer):
    current_dir = os.path.abspath(os.path.curdir)
    working_dir = os.path.join(current_dir, location)
    os.chdir(working_dir)

    subprocess.call(f"{os.path.join(here, 'set_version.sh')} {core} {developer}", shell=True)

    os.chdir(current_dir)


def _process_arguments():
    parser = argparse.ArgumentParser(description="Update all version numbers in the libCellML codebase.",
                                     prefix_chars='/')
    parser.add_argument("directory",
                        help="Relative directory from current working directory"
                             " to directory of libCellML source code.")
    parser.add_argument("core",
                        help="Core version number in the form X.Y.Z where X, Y, and Z are whole numbers.")
    parser.add_argument("developer",
                        nargs="?", default="",
                        help="Developer version number in the form -{dev|rc}.W where W is a whole number.")

    return parser.parse_args()


def main():
    args = _process_arguments()
    run_set_version(args.directory, args.core, args.developer)


if __name__ == "__main__":
    main()
