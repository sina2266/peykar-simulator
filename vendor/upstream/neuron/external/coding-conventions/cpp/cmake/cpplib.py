import argparse
from fnmatch import fnmatch
import functools
import logging
import os
import re
import shlex
import shutil
import subprocess
import sys

THIS_SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))


def log_command(*commands):
    if len(commands) == 1:
        logging.info(" ".join([shlex.quote(e) for e in commands[0]]))
    else:
        logging.info("    " + " |\n    ".join([" ".join(cmd) for cmd in commands]))


def source_dir(git="git"):
    """
    Args:
        git: name or path to Git utility

    Returns:
        absolute path to the root of a repository. The parent repository
        if hpc-coding-conventions is used as a git module, this repository otherwise.

    Alternative to "git rev-parse --show-superproject-working-tree"
    but this solution requires git 2.13 or higher
    """

    def git_rev_parse(*args, **kwargs):
        cmd = list((git, "rev-parse") + args)
        log_command(cmd)
        output = subprocess.check_output(cmd, **kwargs).decode("utf-8").strip()
        return os.path.realpath(output)

    git_dir = git_rev_parse("--git-dir", cwd=THIS_SCRIPT_DIR)
    if os.path.dirname(git_dir) not in THIS_SCRIPT_DIR:
        # This project is used as a git module
        module_dir = git_rev_parse("--show-toplevel", cwd=THIS_SCRIPT_DIR)
        git_dir = git_rev_parse("--git-dir", cwd=os.path.dirname(module_dir))
    return os.path.dirname(git_dir)


class cached_property(object):
    """
    A property that is only computed once per instance and then replaces itself
    with an ordinary attribute. Deleting the attribute resets the property.
    Source: https://github.com/bottlepy/bottle/commit/fa7733e075da0d790d809aa3d2f53071897e6f76
    """  # noqa

    def __init__(self, func):
        self.__doc__ = getattr(func, "__doc__")
        self.func = func

    def __get__(self, obj, cls):
        if obj is None:
            return self

        value = obj.__dict__[self.func.__name__] = self.func(obj)
        return value


def is_file_tracked(file, git="git", cwd=None):
    """
    Args:
        file: relative path to file within a git repository
        cwd: optional path to change before executing the command

    Returns:
        true if the given file is tracked by a git repository, false otherwise
    """
    ret = subprocess.call(
        [git, "ls-files", "--error-unmatch", file],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
        cwd=cwd,
    )
    return ret == 0


def do_merge_yaml(*files, **kwargs):
    """Merge YAML files. The last argument is the destination file"""
    try:
        import yaml
    except ImportError:
        logging.error(
            "Cannot find Python yaml module, which is needed to merge YAML files."
        )
        sys.exit(1)

    succeeded = True
    transformers = kwargs.get("transformers", {})
    out = files[-1]
    ins = files[:-1]

    outdated = not os.path.exists(out) or os.path.getmtime(out) < max(
        (os.path.getmtime(f) for f in ins)
    )
    if outdated:
        data = {}
        for file in ins:
            with open(file) as istr:
                content = yaml.safe_load(istr)
                if not isinstance(content, dict):
                    logging.error(
                        "while reading YAML file %s: expected dictionary but got %s",
                        file,
                        type(content).__name__,
                    )
                    succeeded = False
                    continue

                for key, value in content.items():
                    transform_func = transformers.get(key)
                    if transform_func:
                        data[key] = transform_func(data.get(key), value)
                    else:
                        data[key] = value
        logging.info("writing file %s", out)
        if succeeded:
            with open(out, "w") as ostr:
                yaml.dump(data, ostr, default_flow_style=False)
    else:
        logging.info("file %s is up to date, nothing to do.", out)
    return succeeded


def merge_clang_tidy_checks(orig_checks, new_checks):
    """Merge 2 'Checks' ClangTidy configuration key values"""
    if orig_checks is None:
        return new_checks
    orig_checks = [check.strip() for check in orig_checks.split(",")]
    new_checks = [check.strip() for check in new_checks.split(",")]

    for new_check in new_checks:
        if new_check.startswith("-"):
            name = new_check[1:]
            # remove check when check=google-runtime-references to_=-google-*
            orig_checks = list(
                check for check in orig_checks if not fnmatch(check, name)
            )
            # remove check when check=-google-runtime-references
            # to_=-google-* (simplification)
            orig_checks = list(
                check for check in orig_checks if not fnmatch(check, new_check)
            )
        else:
            # remove check when check=-google-runtime-references to_=google-*
            orig_checks = list(
                check for check in orig_checks if not fnmatch(check, "-" + new_check)
            )
            # remove check when check=google-runtime-references
            # to_=google-* (simplification)
            orig_checks = list(
                check for check in orig_checks if not fnmatch(check, new_check)
            )
        orig_checks.append(new_check)
    return ",".join(orig_checks)


do_merge_clang_tidy_yaml = functools.partial(
    do_merge_yaml, transformers=dict(Checks=merge_clang_tidy_checks)
)


class Tool:
    """Wrapper class for the tools supported by this project
    i.e clang-format cmake-format, and clang-tidy
    """

    def __init__(self, name_or_path):
        """
        Args:
            name_or_path: clang-format, clang-format-13,
            or /path/to/llvm/bin/clang-format-13
        """
        self._name_or_path = name_or_path

    DEFAULT_TOOL_CONFIG = dict(
        config=".{self}",
        custom_config="{self.config}.changes",
        merge_yaml_func=do_merge_yaml,
    )
    TOOL_CONFIG = {
        "cmake-format": dict(
            config=".{self}.yaml",
            custom_config=".{self}.changes.yaml",
        ),
        "clang-tidy": dict(merge_yaml_func=do_merge_clang_tidy_yaml),
    }

    def config_key(self, key):
        """
        retrieve the value of a config key. Looks first into TOOL_CONFIG,
        then DEFAULT_TOOL_CONFIG
        """
        tc = Tool.TOOL_CONFIG.get(str(self), {})
        return tc.get(key, Tool.DEFAULT_TOOL_CONFIG[key])

    @cached_property
    def name(self):
        """
        Returns:
            tool name, i.e "clang-format"
        """
        tool = os.path.basename(self._name_or_path)
        if re.match(".*-[0-9]+", tool):
            tool, _ = tool.rsplit("-", 1)
        return tool

    def __str__(self):
        return self.name

    @property
    def config(self):
        """
        Returns:
            path to the tool config file name, i.e ".clang-format"
        """
        return self.config_key("config").format(self=self)

    @property
    def custom_config(self):
        """
        Returns:
            path to custom tool config file that can contain only modifications
            of the default config, i.e ".clang-format.changes"
        """
        return self.config_key("custom_config").format(self=self)

    @cached_property
    def version(self):
        """
        Returns:
            tool version, i.e "13.0.0"
        """
        cmd = [self._name_or_path, "--version"]
        log_command(cmd)
        proc = subprocess.run(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            check=True,
            encoding="utf-8",
        )
        output = proc.stdout.strip()
        match = re.search("([0-9]+\\.[0-9]+\\.[0-9]+)", output)
        if match:
            ver = match.group(1)
            return ver
        raise RuntimeError(
            f"Could not extract {self.name} version from output: '{output}'"
        )

    @cached_property
    def bbp_config(self):
        """
        Returns:
            absolute path to the proper config file
            in the hpc-coding-conventions project
        """
        version = self.version
        major_ver, _ = self.version.split(".", 1)
        config_lib_dir = os.path.dirname(THIS_SCRIPT_DIR)
        test_ver = int(major_ver)
        while test_ver >= 0:
            candidate = os.path.join(config_lib_dir, f"{self.name}-{test_ver}")
            if os.path.exists(candidate):
                return candidate
            test_ver -= 1
        candidate = os.path.join(config_lib_dir, self.config[1:])
        if os.path.exists(candidate):
            return candidate
        raise RuntimeError(
            f"Could not find appropriate config file for {self} {version}"
        )

    def setup_config(self, source_dir, git="git"):
        """
        Build the tool config file and write it in the project source directory

        Args:
            source_dir: project top directory where the config file should be written
        """
        config = os.path.join(source_dir, self.config)
        if not is_file_tracked(
            self.config, git=git, cwd=source_dir
        ) or not os.path.exists(config):
            custom_config = os.path.join(source_dir, self.custom_config)
            if os.path.exists(custom_config):
                logging.info(f"Merging custom {self} YAML changes ")
                self.config_key("merge_yaml_func")(
                    self.bbp_config, custom_config, config
                )
            else:
                bbp_config = self.bbp_config
                bbp_config_base = os.path.basename(bbp_config)
                if not os.path.exists(config) or os.path.getmtime(
                    config
                ) < os.path.getmtime(bbp_config):
                    logging.info(
                        f"Copying BBP config {bbp_config_base} to {source_dir}"
                    )
                    shutil.copy(bbp_config, config)
                else:
                    logging.info(
                        f"{self} config is up to date with BBP {bbp_config_base} config"
                    )
        else:
            logging.info(f"{self} config is tracked by git, nothing to do.")


def make_file_filter(excludes_re, files_re):
    """
    Returns:
        a Python function used to filter the C++ files that needs to
        be formatted.
    """

    def _func(cpp_file):
        for exclude_re in excludes_re:
            if exclude_re.match(cpp_file):
                return True
        for file_re in files_re:
            if file_re.match(cpp_file):
                return False
        return True

    return _func


def collect_files(source_dir, filter_file):
    """
    Args:
        cli_args: parsed CLI options
        filter_file: a function returning `True` if the given file should be
        excluded, `False` otherwise.
    Returns:
        Generator of path to files in source_dir
    """

    cmd = ["git", "ls-tree", "-r", "-z", "--name-only", "--full-name", "HEAD"]
    log_command(cmd)
    files = subprocess.check_output(cmd, cwd=source_dir).decode("utf-8").split("\0")
    files = [x for x in files if not filter_file(x)]
    files = [os.path.join(source_dir, x) for x in files]
    return files


def parse_cli(
    choices=None, description=None, parser_args=None, args=None, executable=None
):
    """Common CLI parser for all tool wrapper scripts bbp-{tool}.py"""
    if executable is None:
        # deduce tool name from the Python script name i.e
        # /path/to/bbp-clang-format.py => clang-format
        script_name = os.path.basename(sys.argv[0])
        if re.match("bbp-.*\\.py", script_name):
            executable = script_name[4:-3]
        else:
            raise Exception(f"Could not extract tool name from script: {sys.argv[0]}")

    choices = (choices or []).append("config")
    parser = argparse.ArgumentParser(
        description=description or "Wrapper for checker utility"
    )
    parser.add_argument(
        "-S",
        dest="source_dir",
        metavar="PATH",
        help="Path to CMake source directory, default is the parent repository root",
    )
    parser.add_argument(
        "--executable", help="Path to executable to run", default=executable
    )
    parser.add_argument(
        "--excludes-re",
        nargs="*",
        default=[],
        help="list of regular expressions of files to exclude",
    )
    parser.add_argument(
        "--files-re",
        nargs="*",
        default=[],
        help="List of regular expressions of files to include",
    )
    parser.add_argument(
        "--files-by-suffix", nargs="*", help="List of suffixes of the files to include"
    )
    parser.add_argument(
        "--make-unescape-re",
        action="store_true",
        help="Unescape make-escaped regular-expression arguments",
    )
    parser.add_argument(
        "--git-executable",
        default="git",
        help="Path to git executable [default is %(default)s]",
    )

    for name, kwargs in parser_args or []:
        parser.add_argument(name, **kwargs)
    parser.add_argument("action", choices=choices)
    parser.add_argument("options", nargs="*", help="Options given to executable")
    result = parser.parse_args(args=args)
    if not result.source_dir:
        result.source_dir = source_dir(result.git_executable)
    if result.make_unescape_re:

        def make_unescape_re(pattern):
            if pattern.endswith("$$"):
                pattern = pattern[:-1]
            pattern = pattern.replace("\\\\", "\\")
            return pattern

        def make_unescape_res(patterns):
            return [make_unescape_re(pattern) for pattern in patterns]

        result.files_re = make_unescape_res(result.files_re)
        result.excludes_re = make_unescape_res(result.excludes_re)

    if result.files_by_suffix:
        result.files_re += [f".*\\.{suffix}$" for suffix in result.files_by_suffix]

    result.options = [opt for opt in result.options if opt]

    Tool(result.executable).setup_config(result.source_dir, result.git_executable)
    if result.action == "config":
        sys.exit(0)

    return result


def main(args=None):
    parser = argparse.ArgumentParser(description="Utility program")
    subparsers = parser.add_subparsers(help="sub-command help")
    merge_yaml = subparsers.add_parser("merge-yaml", help="Merge yaml files")
    merge_yaml.add_argument("files", nargs="+", help="input files then output file")
    merge_yaml.set_defaults(func=do_merge_yaml)

    merge_yaml = subparsers.add_parser(
        "merge-clang-tidy-config", help="Merge ClangTidy configuration files"
    )
    merge_yaml.add_argument("files", nargs="+", help="input files then output file")
    merge_yaml.set_defaults(func=do_merge_clang_tidy_yaml)

    result = parser.parse_args(args=args)
    return result.func(*result.files)


if __name__ == "__main__":
    level = logging.INFO if "VERBOSE" in os.environ else logging.WARN
    logging.basicConfig(level=level, format="%(levelname)s: %(message)s")
    sys.exit(0 if main() else 1)
