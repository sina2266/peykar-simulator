import argparse
import os
import shutil

surplus_directories = ["./_sources", "./_static/css", "./_static/js", "./.doctrees"]


def remove_surplus_sphinx_files(location):
    current_dir = os.path.abspath(os.path.curdir)
    working_dir = os.path.join(current_dir, location)
    os.chdir(working_dir)

    for root, dirs, files in os.walk(".", topdown=False):
        if root in surplus_directories:
            shutil.rmtree(root)
        else:
            for name in files:
                parts = os.path.splitext(name)
                if name == "searchindex.js":
                    os.remove(os.path.join(root, name))
                elif name == "objects.inv":
                    os.remove(os.path.join(root, name))
                elif len(parts) == 2 and parts[1] == ".html":
                    os.remove(os.path.join(root, name))
                elif parts[-1] == ".css" and root == "./_static":
                    os.remove(os.path.join(root, name))
                elif parts[-1] == ".js" and root == "./_static":
                    os.remove(os.path.join(root, name))

    os.chdir(current_dir)


def _process_arguments():
    parser = argparse.ArgumentParser(description="Remove all build files not needed for rendering documentation with vue3-sphinx-xml.")
    parser.add_argument("directory",
                        help="Relative directory from current working directory"
                             " to directory to remove html files from.")

    return parser.parse_args()


def main():
    args = _process_arguments()
    remove_surplus_sphinx_files(args.directory)


if __name__ == "__main__":
    main()
