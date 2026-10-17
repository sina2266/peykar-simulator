"""
Main module of this project
"""

import abc
import argparse
import collections
import copy
from fnmatch import fnmatch
import functools
import importlib.metadata
import importlib.util
import logging
import operator
import os
import os.path
from pathlib import Path
import re
import shlex
import shutil
import subprocess
import sys
import tempfile
import urllib.request
import venv

THIS_SCRIPT_DIR = Path(__file__).resolve().parent
DEFAULT_RE_EXTRACT_VERSION = "([0-9]+\\.[0-9]+(\\.[0-9]+)?[ab]?)"


def forget_python_pkg(package):
    """
    exclude from PYTHONPATH environment variable
    and sys.path trace of the specified package

    Args:
        package: name of the Python package to exclude
    """
    try:
        dist = importlib.metadata.distribution(package)
    except importlib.metadata.PackageNotFoundError:
        return

    dist_location = None
    if dist.files:
        # Get the parent directory of the first file in the distribution
        first_file = next(iter(dist.files))
        dist_location = str(first_file.locate().parent.parent)
    else:
        # Fallback: try to get location from sys.path and package name
        try:
            spec = importlib.util.find_spec(package)
            if spec and spec.origin:
                # Get the parent directory containing the package
                dist_location = str(Path(spec.origin).parent.parent)
        except (AttributeError, ImportError):
            pass

    if dist_location is None:
        return

    PYTHONPATH = os.environ.get("PYTHONPATH")
    if PYTHONPATH is not None and dist_location in PYTHONPATH:
        logging.debug(
            "Remove incompatible version of %s in PYTHONPATH: %s",
            package,
            dist_location,
        )
        os.environ["PYTHONPATH"] = PYTHONPATH.replace(dist_location, "")
        try:
            sys.path.remove(dist_location)
        except ValueError:
            pass


@functools.lru_cache()
def source_dir():
    """
    Returns:
        absolute path to the root of a git repository.
        The parent repository if hpc-coding-conventions is used
        as a git module, this repository otherwise.

    Implementation note:
        alternative is to use
        "git rev-parse --show-superproject-working-tree"
        but this solution requires git 2.13 or higher
    """

    def git_rev_parse(*args, **kwargs):
        cmd = list((which("git"), "rev-parse") + args)
        log_command(cmd, level=logging.DEBUG)
        output = subprocess.check_output(cmd, **kwargs).decode("utf-8").strip()
        return Path(output).resolve()

    git_dir = Path(git_rev_parse("--git-dir", cwd=THIS_SCRIPT_DIR))
    if git_dir.parent not in THIS_SCRIPT_DIR.parents:
        # This project is used as a git module
        module_dir = git_rev_parse("--show-toplevel", cwd=THIS_SCRIPT_DIR)
        git_dir = git_rev_parse("--git-dir", cwd=os.path.dirname(module_dir))
        try:
            Path.cwd().relative_to(module_dir)
            # cwd is inside hpc-coding-conventions module.
            # assume this is for its development.
            return module_dir
        except ValueError:
            pass
    return git_dir.parent


def merge_yaml_files(*files, **kwargs):
    """Merge YAML files. The last argument is the destination file"""
    try:
        import yaml  # pylint: disable=C0415
    except ImportError:
        logging.error(
            "Cannot find Python yaml module, which is needed to merge YAML files."
        )
        sys.exit(1)

    succeeded = True
    transformers = kwargs.get("transformers") or {}
    out = files[-1]
    ins = files[:-1]

    outdated = not os.path.exists(out) or os.path.getmtime(out) < max(
        (os.path.getmtime(f) for f in ins)
    )
    if outdated:
        data = {}
        for file in ins:
            with open(file, encoding="utf-8") as istr:
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
            with open(out, "w", encoding="utf-8") as ostr:
                yaml.dump(data, ostr, default_flow_style=False)
    else:
        logging.info("file %s is up to date, nothing to do.", out)
    return succeeded


def is_file_tracked(file: str, cwd=None) -> bool:
    """
    Args:
        file: relative path to file within a git repository
        cwd: optional path to change before executing the command

    Returns:
        true if the given file is tracked by a git repository, false otherwise
    """
    ret = subprocess.call(
        [which("git"), "ls-files", "--error-unmatch", file],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
        cwd=cwd,
    )
    return ret == 0


def chunkify(seq, chunk_size=1):
    """
    Split a sequence of elements into chunks of a given size

    Args:
        seq: sequence of elements
        chunk_size: maximum chunk size

    Returns:
        a generator of sequence
    """
    for chunk_start_idx in range(0, len(seq), chunk_size):
        yield seq[chunk_start_idx : chunk_start_idx + chunk_size]


def log_command(*commands, logger=None, level=logging.DEBUG):
    """
    Utility function to report to the logger a shell command about to be executed.
    """
    if len(commands) == 1:
        message = " ".join([shlex.quote(e) for e in commands[0]])
    else:
        message = "    " + " |\n    ".join([" ".join(cmd) for cmd in commands])
    logger = logging if logger is None else logger
    logger.log(level, message)


class cached_property:  # pylint: disable=C0103
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


@functools.lru_cache()
def which(program: str, paths=None):
    """
    Find the first location of a program in PATH environment variable.

    Args:
        program: program to look for i.e "clang-format"
        paths: optional list of paths where to look for the program.
               default is the PATH environment variable.

    Return:
        Path `str` of the executable if found, `None otherwise
    """
    env_variable = re.sub("[^0-9a-zA-Z_]", "_", program).upper()

    program = Path(os.environ.get(env_variable, program))
    if program.is_absolute():
        return str(program)

    paths = paths or os.getenv("PATH").split(os.path.pathsep)
    for path in paths:
        abs_path = Path(path).joinpath(program)
        if abs_path.exists() and os.access(abs_path, os.X_OK):
            return str(abs_path)


def where(program: str, regex=None, exclude_regex=None, paths=None):
    """
    Find all the locations of a program in PATH environment variable.

    Args:
        program: program to look for i.e "clang-format"
        regex: optional regular expression for alternative program names
               i.e re.compile("clang-format-.*"])
        exclude_regex: optional regular expression to exclude alternate
                       program names matching `regex`.
        paths: optional list of paths where to look for the program.
               default is the PATH environment variable.
    """
    env_variable = re.sub("[^0-9a-zA-Z_]", "_", program).upper()

    program = os.environ.get(env_variable, program)
    if os.path.isabs(program):
        yield program
        return

    paths = paths or os.getenv("PATH").split(os.path.pathsep)
    for path in paths:
        abs_path = os.path.join(path, program)
        if os.path.exists(abs_path) and os.access(abs_path, os.X_OK):
            yield abs_path
        if regex and os.path.exists(path):
            for file_name in os.listdir(path):
                file_path = os.path.join(path, file_name)
                if regex.match(file_name) and os.access(file_path, os.X_OK):
                    if not exclude_regex or not exclude_regex.match(file_name):
                        yield file_path


class BBPVEnv:
    """
    Wrapper for the Python virtual environment used by this module.
    """

    def __init__(self, path: str):
        """
        Args:
            path: path to virtual environment
        """
        assert isinstance(path, Path)
        self._path = path
        os.environ["PIP_DISABLE_PIP_VERSION_CHECK"] = "1"

    @property
    def path(self) -> str:
        """
        Return:
            Path to the virtual environment
        """
        return self._path

    @property
    def bin_dir(self) -> str:
        """
        Return:
            Path to the /bin directory of the virtual environment
        """
        return self.path.joinpath("bin")

    @property
    def interpreter(self) -> str:
        """
        Return:
            Path to the Python interpreter of the virtual environment
        """
        return self.bin_dir.joinpath("python")

    def pip_cmd(self, *args) -> list[str]:
        """
        Execute a pip command

        Arguments:
            args: the command

        Return:
            array containing the shell command to execute
        """
        return [str(self.interpreter), "-m", "pip"] + list(args)

    def pip_install(self, requirement, upgrade=False):
        """
        Args:
            requirement: list of (str or Requirement) to install. Possible inputs:
                - pkg_resources.Requirement.parse("foo==1.0")
                - "foo==1.0"
                - [pkg_resources.Requirement.parse("foo==1.0"), "bar==1.0"]
        """
        cmd = self.pip_cmd("install")
        if logging.getLogger().level != logging.DEBUG:
            cmd += ["-q"]
        if upgrade:
            cmd += ["--upgrade"]
        if not isinstance(requirement, list):
            requirement = [requirement]
        requirement = [str(req) for req in requirement]
        cmd += requirement
        log_command(cmd)
        subprocess.check_call(cmd)

    def ensure_pip(self):
        def py_call(*cmd, check=False):
            cmd = [str(self.interpreter)] + list(cmd)
            log_command(cmd)
            kwargs = {}
            if logging.getLogger().level != logging.DEBUG:
                kwargs.update(stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            call = subprocess.call
            if check:
                call = subprocess.check_call
            return call(cmd, **kwargs) == 0

        if not py_call("-m", "pip", "--version"):
            py_call("-m", "ensurepip", "--default-pip")
            if not py_call("-m", "pip", "--version"):
                with tempfile.NamedTemporaryFile(suffix=".py") as get_pip_script:
                    url = "https://bootstrap.pypa.io/get-pip.py"
                    urllib.request.urlretrieve(url, get_pip_script.name)
                    py_call(get_pip_script.name)
                py_call("-m", "pip", "--version", check=True)

    @property
    def in_venv(self) -> bool:
        """
        Return:
            True if the current process is run by the Python interpreter
            of this virtual environment, False otherwise.
        """
        venv_python_interpreter_pattern = self.bin_dir.resolve().joinpath("python*")
        return fnmatch(sys.executable, venv_python_interpreter_pattern)

    def restart_in_venv(self, reason=""):
        """
        Replace the current process by the execution of the same command but with
        the Python interpreter of this virtual environment.

        This function doesn't return since the process is entirely replaced.

        Args:
            reason: optional log message
        """
        if not self.in_venv:
            if not self.path.is_dir():
                builder = venv.EnvBuilder(symlinks=True, with_pip=True)
                logging.debug("Creating virtual environment %s", self.path)
                builder.create(str(self.path))
                self.ensure_pip()
                self.pip_install("pip", upgrade=True)
        logging.debug("Restarting process within own Python virtualenv %s", reason)
        os.execv(self.interpreter, [self.interpreter] + sys.argv)

    def is_requirement_met(self, requirement) -> bool:
        """
        Args:
            requirement: str of pkg_resources.Requirement

        Return:
            True if the current Python environment fulfills the given requirement,
            False otherwise
        """
        try:
            import packaging.requirements

            # Parse the requirement string
            req = packaging.requirements.Requirement(str(requirement))

            # Check if the package is installed
            try:
                dist = importlib.metadata.distribution(req.name)
            except importlib.metadata.PackageNotFoundError:
                return False

            # Check if the version satisfies the requirement
            if req.specifier and not req.specifier.contains(dist.version):
                return False

            return True
        except ImportError:
            self._install_requirement("packaging", restart=True)
        except Exception:
            # Handle any parsing or version checking errors
            return False

    def ensure_requirement(self, requirement, restart=True):
        """
        Ensure the Python script is running in an environment fulfilling
        the given requirement.

        If requirement already met
        Then do nothing.
        Else
            If not running in the virtual environment
            Then
                Create the virtual environment If it doesn't exist
                Rerun the Python script within the virtual environment
            Else
                If requirement is not met
                Then
                    install the package
                    Return the Python script only if "restart" is True

        Args:
            requirement: str of pkg_resources.Requirement
            restart: whether this process may restart after the requirement
                is installed.
        """
        if not self.is_requirement_met(requirement):
            self._install_requirement(requirement, restart)

    def _install_requirement(self, requirement, restart=False):
        """
        Force installation of the requirement in the virtual environment

        Args:
            requirement: str of pkg_resources.Requirement
            restart: whether this process may restart after the requirement
                is installed.
        """
        if self.in_venv:
            self.pip_install(requirement)
            if restart:
                self.restart_in_venv(
                    f"to take into account installed requirement {requirement}"
                )
        else:
            self.restart_in_venv(f"because requirement {requirement} is not met")


class Tool(metaclass=abc.ABCMeta):
    """
    Wrapper class over a third-party tool like clang-format or pre-commit
    """

    LOG_JOBS = True

    def __init__(self, config: dict, user_config: dict):
        """
        Args:
            config: describes how to tool works internally
            user_config: describes how to use it
        """
        self._config = config
        self._user_config = user_config

    @staticmethod
    @functools.lru_cache()
    def job_logger() -> logging.Logger:
        """
        Return:
            `logging.getLogger` instance use to report
            the commands executed by a tool.
        """
        logger = logging.getLogger("job")
        logger.propagate = False
        logger.setLevel(logging.INFO if Tool.LOG_JOBS else logging.WARN)
        handler = logging.StreamHandler(sys.stdout)
        formatter = logging.Formatter("%(message)s")
        handler.setFormatter(formatter)
        logger.addHandler(handler)
        return logger

    @property
    def name(self) -> str:
        """
        Return:
            the tool name
        """
        return self._config["name"]

    def __str__(self) -> str:
        """
        Return:
            the tool name
        """
        return self.name

    @property
    def config(self) -> dict:
        """
        Return:
            tool internal configuration
        """
        return self._config

    @property
    def user_config(self) -> dict:
        """
        Return:
            tool user configuration
        """
        return self._user_config

    @property
    def path(self) -> str:
        """
        Return:
            absolute path to the tool
        """
        path = self._user_config.get("path")
        if path is None:
            path = getattr(self, "_path", None)
            if path is None:
                raise RuntimeError(
                    f"{self.__class__}.configure should be called before"
                )
        return path

    @cached_property
    def requirement(self):
        """
        Return:
            `pkg_resources.Requirement` of the tool if it is a Python package,
            `None` otherwise
        """
        pip_pkg = self.config["capabilities"].pip_pkg
        assert isinstance(pip_pkg, (str, bool))
        if isinstance(pip_pkg, str):
            name = pip_pkg
        else:
            name = self.name

        import packaging.requirements

        return packaging.requirements.Requirement(
            f"{name} {self.user_config['version']}"
        )

    @abc.abstractmethod
    def configure(self):
        """
        Find the tool on the system and setup its configuration
        """

    @classmethod
    def cli_options(cls, task: str, parser: argparse.ArgumentParser):
        """
        Hook function to add options to the CLI parser of a task

        Args:
            task: task name
            parser: argument parser to complete
        """

    def cmd_opts(self, task: str, dry_run=False, **kwargs):
        """
        Args:
            task: task name
            dry_run: whether the tool should actually not perform the task
            kwargs: additional options given in CLI

        Return:
            The command line options to pass to the tool
        """
        task_config = self.config["provides"][task]
        if dry_run:
            try:
                return task_config["dry_run_cmd_opts"]
            except KeyError as exc:
                raise Exception(f"{self}: error: dry-run: unsupported option") from exc
        else:
            return task_config["cmd_opts"]

    @cached_property
    def bbp_config_file(self):
        """
        Returns:
            absolute path `pathlib.Path` to the proper config file
            in the hpc-coding-conventions project.
            It depends on the version of the tool
        """
        version = self._version
        major_ver, _ = self._version.split(".", 1)
        config_lib_dir = THIS_SCRIPT_DIR
        test_ver = int(major_ver)
        while test_ver >= 0:
            candidate = config_lib_dir.joinpath(f"{self}-{test_ver}")
            if candidate.exists():
                return candidate
            test_ver -= 1
        candidate = config_lib_dir.joinpath(
            self.config["config_file"].format(self=self)[1:]
        )
        if candidate.exists():
            return candidate
        raise RuntimeError(
            f"Could not find appropriate config file for {self} {version}"
        )

    def prepare_config(self):
        """
        Setup the configuration file of the tool. For instance this function
        will create the proper ".clang-format" file at the root of a C++ project.
        """
        config_fname = self.config["config_file"].format(self=self)
        config_f = source_dir().joinpath(config_fname)
        if not is_file_tracked(config_fname, cwd=source_dir()) or not config_f.exists():
            custom_conf_f_name = self.config["custom_config_file"].format(self=self)
            custom_config_f = source_dir().joinpath(custom_conf_f_name)
            if custom_config_f.exists():
                build_file = False
                if not config_f.exists():
                    build_file = True
                else:
                    config_f_mtime = os.path.getmtime(config_f)
                    deps = [config_f, custom_config_f]
                    if config_f_mtime < max((os.path.getmtime(f) for f in deps)):
                        build_file = True
                if build_file:
                    logging.info("Merging custom %s YAML changes ", self)
                    merge_yaml_files(
                        self.bbp_config_file,
                        custom_config_f,
                        config_f,
                        transformers=self.config.get("config_yaml_transformers"),
                    )
                else:
                    logging.info(
                        "%s config is up to date with BBP %s"
                        " and custom %s config files",
                        self,
                        self.bbp_config_file.name,
                        custom_conf_f_name,
                    )
            else:
                bbp_config_f = self.bbp_config_file
                bbp_config_base = bbp_config_f.name
                if not config_f.exists() or os.path.getmtime(
                    config_f
                ) < os.path.getmtime(bbp_config_f):
                    logging.info(
                        "Copying BBP config %s to %s", bbp_config_base, source_dir()
                    )
                    shutil.copy(bbp_config_f, config_f)
                else:
                    logging.info(
                        "%s config is up to date with BBP %s config",
                        self,
                        bbp_config_base,
                    )
        else:
            logging.info("%s config is tracked by git, nothing to do.", self)

    def run(self, task: str, *files, dry_run=False, **kwargs):
        """
        Execute a task on a set of files

        Args:
            task: task name
            files: list of files on which to execute the task
            dry_run: if True, the tool should not actually perform the task,
                     just report issues.
            kwargs: additional task options

        Return:
            number of failed tasks
        """
        max_num_files = self.config["capabilities"].cli_max_num_files
        num_errors = 0
        for files_chunk in chunkify(files, max_num_files):
            num_errors += self._run_chunk(task, *files_chunk, dry_run=dry_run, **kwargs)
        return num_errors

    def _run_chunk(self, task: str, *files, cwd=None, **kwargs):
        cmd = [self.path]
        user_option = self.user_config.get("option") or []
        if isinstance(user_option, str):
            user_option = [user_option]
        cmd += user_option

        dry_run = kwargs.get("dry_run", False)

        task_config = self.config["provides"][task]
        call_kwargs = dict(cwd=cwd)
        if logging.getLogger().level > logging.INFO:
            call_kwargs.update(stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        cmd += self.cmd_opts(task, **kwargs) + list(files)
        log_command(cmd, logger=self.job_logger(), level=logging.INFO)
        status = subprocess.call(cmd, **call_kwargs)
        if dry_run and status != 0:
            lang_str = "/".join(task_config["languages"])
            logging.error(
                "%s | failed checks (one or more): %s",
                lang_str,
                " ".join(files),
            )
        return 1 if status != 0 else 0

    def accepts_file(self, file: str) -> bool:
        """
        Args:
            file: path to file

        Return:
            True if the tool accepts the file, False otherwise
        """
        for regexp in self.user_config.get("exclude", {}).get("match", []):
            if regexp.match(file):
                return False
        for regexp in self.user_config.get("include", {}).get("match", []):
            if regexp.match(file):
                return True
        return False


class ExecutableTool(Tool):
    """
    Specialization of `Tool` for utilities that can be called from the command line
    """

    def configure(self):
        if self.user_config.get("path") is None:
            # path is not provided by the user conf
            # let's find it!
            try:
                if self.user_config.get("requirements", []):
                    raise FileNotFoundError(
                        f"Force usage of custom virtualenv for {self} "
                        "since it requires extra Python packages"
                    )
                self._path, self._version = self.find_tool_in_path()
            except FileNotFoundError as e:
                if self.requirement:
                    forget_python_pkg(self.requirement.name)
                    BBPProject.virtualenv().ensure_requirement(self.requirement)
                    self._path, self._version = self.find_tool_in_path(
                        [BBPProject.virtualenv().bin_dir]
                    )
                else:
                    raise e
            logging.info(
                f"{self}: found %s (%s) matching requirement %s",
                self._path,
                self._version,
                self.requirement,
            )
        else:
            self._version = self.find_version(self.path)
        # Install additional requirements
        for req in self.user_config.get("requirements", []):
            BBPProject.virtualenv().ensure_requirement(req, restart=False)

    def find_tool_in_path(self, search_paths=None):
        paths = list(
            where(self.name, self.names_regex, self.names_exclude_regex, search_paths)
        )
        if not paths:
            raise FileNotFoundError(f"Could not find tool {self}")
        all_paths = [(p, self.find_version(p)) for p in paths]
        paths = list(
            filter(lambda tpl: self.requirement.specifier.contains(tpl[1]), all_paths)
        )
        paths = list(sorted(paths, key=lambda tup: tup[1]))  # sort by version
        if not paths:
            raise FileNotFoundError(
                f"Could not find a version of {self} "
                + f"matching the requirement {self.requirement}\nCandidates are:\n"
                + "\n".join(f"{tpl[1]}: {tpl[0]}" for tpl in all_paths)
            )
        return paths[-1]

    @cached_property
    def names_regex(self):
        """
        Return:
            Optional regular expression to look for
            the tool in PATH environment variable.
        """
        pattern = self.config.get("names_regex")
        return re.compile(pattern) if pattern else None

    @cached_property
    def names_exclude_regex(self):
        """
        Return:
            Optional regular expression to exclude executables
            for the tool in PATH.
        """
        pattern = self.config.get("names_exclude_regex")
        return re.compile(pattern) if pattern else None

    def find_version(self, path: str) -> str:
        """
        Returns:
            extract version of given utility, i.e "13.0.0"
        """
        if self.config["capabilities"].pip_pkg:
            # This tool is a Python package
            venv = BBPProject.virtualenv()
            if venv.in_venv and venv.bin_dir in Path(path).parents:
                # `path` belongs to the ClangFormat Python package
                # available in the current Python environment.
                # Let's query the environment instead of parsing
                # the output of `clang-format --version`
                pkg_name = self.name
                if isinstance(self.config["capabilities"].pip_pkg, str):
                    pkg_name = self.config["capabilities"].pip_pkg

                import packaging.requirements

                req = packaging.requirements.Requirement(pkg_name)
                return importlib.metadata.version(req.name)

        cmd = [path] + self._config["version_opt"]
        log_command(cmd)
        proc = subprocess.run(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            check=False,
            encoding="utf-8",
        )
        output = proc.stdout.strip()
        match = re.search(self._config["version_re"], output)
        if match:
            ver = match.group(1)
            return ver
        raise RuntimeError(
            f"Could not extract version of program {path} from output: '{output}'"
        )


class ClangTidy(ExecutableTool):
    """
    Specialization for ClangTidy utility in order to:
    - add the -p CLI option and takes it into account when
      executing the command
    - properly merge the ClangTidy checks in YAML files
    """

    @classmethod
    def cli_options(cls, task: str, parser: argparse.ArgumentParser):
        parser.add_argument(
            "-p",
            metavar="build-path",
            dest="compile_commands_file",
            type=str,
            help="a Clang compile command database",
        )

    def cmd_opts(self, task: str, compile_commands_file=None, **kwargs):
        compile_commands_file = compile_commands_file or self.user_config.get(
            "compile_commands_file"
        )
        if compile_commands_file:
            return ["-p", compile_commands_file]
        return []

    @classmethod
    def merge_clang_tidy_checks(cls, orig_checks, new_checks):
        """Merge 2 'Checks' ClangTidy configuration key values"""
        if orig_checks is None:
            return new_checks
        if new_checks is None:
            return orig_checks
        orig_checks = [check.strip() for check in orig_checks.split(",")]
        new_checks = [check.strip() for check in new_checks.split(",")]

        for new_check in new_checks:
            name_without_prefix = (
                new_check[1:] if new_check.startswith("-") else new_check
            )
            name_with_prefix = "-" + name_without_prefix
            orig_checks = list(
                check for check in orig_checks if not fnmatch(check, name_with_prefix)
            )
            orig_checks = list(
                check
                for check in orig_checks
                if not fnmatch(check, name_without_prefix)
            )
            orig_checks.append(new_check)
        return ",".join(c for c in orig_checks if c)


class TaskDescription(
    collections.namedtuple(
        "TaskDescription", ["on_codebase", "modify_files", "description"]
    )
):
    """
    Attributes:
        on_codebase: True if the tool accepts files in parameter (like clang-format),
                     False otherwise (like pre-commit)
        modify_files: True if the tool can modify files, False otherwise.
        description: Tool short description
    """


class ToolCapabilities(
    collections.namedtuple(
        "ToolCapabilities",
        ["cli_accept_dir", "cli_max_num_files", "pip_pkg"],
    )
):
    """
    Attributes:
        cli_accept_dir: should directories be passed directly to the tool (like black)
                        or should the files be listed and then passed to the tool
                        (like clang-format)
        cli_max_num_files: number of files that can be passed at once to the tool in CLI
        pip_pkg:
            - `True` if the tool is an installable Python package
            - the package name if different than the tool
            - `False` if the tool is not a Python package
    """


class BBPProject:
    # This is the config file of the HPC Coding Conventions project.
    CONFIG_FILE = "bbp-project.yaml"
    # This is the config file for the project being formatted. The path is
    # relative to root of the project using the HPC CC.
    USER_CONFIG_FILE = ".bbp-project.yaml"
    TASKS_DESCRIPTION = {
        "format": TaskDescription(
            on_codebase=True,
            modify_files=True,
            description="Code formatter utility",
        ),
        "static-analysis": TaskDescription(
            on_codebase=True,
            modify_files=False,
            description="Code static analyzer",
        ),
        "clang-tidy": TaskDescription(
            on_codebase=True,
            modify_files=False,
            description="C++ code static analyzer",
        ),
    }
    TOOLS_DESCRIPTION = dict(
        ClangFormat=dict(
            cls=ExecutableTool,
            name="clang-format",
            names_regex="clang-format-[-a-z0-9]+$",
            names_exclude_regex="clang-format-.*diff.*$",
            version_opt=["--version"],
            version_re=DEFAULT_RE_EXTRACT_VERSION,
            capabilities=ToolCapabilities(
                cli_accept_dir=False,
                cli_max_num_files=30,
                pip_pkg=True,
            ),
            provides=dict(
                format=dict(
                    languages=["C++"],
                    cmd_opts=["-i"],
                    dry_run_cmd_opts=["--dry-run", "--ferror-limit", "1", "--Werror"],
                )
            ),
            config_file=".{self}",
            custom_config_file=".{self}.changes",
        ),
        CMakeFormat=dict(
            cls=ExecutableTool,
            name="cmake-format",
            version_opt=["--version"],
            version_re=DEFAULT_RE_EXTRACT_VERSION,
            provides=dict(
                format=dict(
                    languages=["CMake"],
                    cmd_opts=["-i"],
                    dry_run_cmd_opts=["--check"],
                )
            ),
            capabilities=ToolCapabilities(
                cli_accept_dir=False,
                cli_max_num_files=30,
                pip_pkg="cmake-format[YAML]",
            ),
            config_file=".{self}.yaml",
            custom_config_file=".{self}.changes.yaml",
        ),
        ClangTidy=dict(
            cls=ClangTidy,
            name="clang-tidy",
            names_regex="clang-tidy-.*",
            version_opt=["--version"],
            version_re=DEFAULT_RE_EXTRACT_VERSION,
            provides={
                "static-analysis": dict(languages=["C++"]),
                "clang-tidy": dict(
                    languages=["C++"],
                ),
            },
            capabilities=ToolCapabilities(
                cli_accept_dir=False, cli_max_num_files=30, pip_pkg=False
            ),
            config_file=".{self}",
            custom_config_file=".{self}.changes.yaml",
            config_yaml_transformers=ClangTidy.merge_clang_tidy_checks,
        ),
        Flake8=dict(
            cls=ExecutableTool,
            name="flake8",
            version_opt=["--version"],
            version_re=DEFAULT_RE_EXTRACT_VERSION,
            provides={
                "static-analysis": dict(
                    languages=["Python"],
                    cmd_opts=[],
                ),
            },
            capabilities=ToolCapabilities(
                cli_accept_dir=True,
                cli_max_num_files=30,
                pip_pkg=True,
            ),
        ),
        Black=dict(
            cls=ExecutableTool,
            name="black",
            version_opt=["--version"],
            version_re=DEFAULT_RE_EXTRACT_VERSION,
            provides=dict(
                format=dict(
                    languages=["Python"],
                    cmd_opts=[],
                    dry_run_cmd_opts=["--check", "--diff", "--color"],
                )
            ),
            capabilities=ToolCapabilities(
                cli_accept_dir=True,
                cli_max_num_files=30,
                pip_pkg=True,
            ),
        )
        # Will come later
        # PreCommit=dict(
        #     cls=ExecutableTool,
        #     name="pre-commit",
        #     version_opt=["--version"],
        #     version_re="([0-9]+\\.[0-9]+\\.[0-9]+)",
        #     provides={"setup": dict()},
        #     capabilities=ToolCapabilities(
        #     ),
        # ),
    )

    @classmethod
    @functools.lru_cache()
    def virtualenv(cls):
        return BBPVEnv(source_dir().joinpath(".bbp-project-venv"))

    @classmethod
    def task_cli(cls, task: str):
        """
        Construct the `argparse.ArgumentParser` for the given `task`

        Args:
            task: task name to execute

        Return:
            instance of argparse.ArgumentParser
        """
        task_config = cls.TASKS_DESCRIPTION[task]
        parser = argparse.ArgumentParser(description=task_config.description)
        ns = argparse.Namespace()
        if task_config.modify_files:
            parser.add_argument(
                "-n",
                "--dry-run",
                action="store_true",
                help="do not update the files, simply report formatting issues",
            )

        if task_config.on_codebase:
            supported_languages = cls.supported_languages(task)
            if not supported_languages:
                raise RuntimeError(f"No tool supports task named '{task}'")
            if len(supported_languages) > 1:
                parser.add_argument(
                    "--lang",
                    action="store",
                    dest="languages",
                    help="only format the specified languages, "
                    "default is: '%(default)s'",
                    default=",".join(map(str.lower, supported_languages)),
                )
            else:
                ns.languages = next(iter(supported_languages))
            parser.add_argument(
                "sources",
                metavar="SRC",
                nargs="*",
                help="Files or directories. Default is the entire codebase.",
            )

        for tool in cls.TOOLS_DESCRIPTION.values():
            task_info = tool["provides"].get(task)
            if task_info is not None:
                tool["cls"].cli_options(task, parser)

        parser.add_argument(
            "-v",
            "--verbose",
            action="count",
            default=0,
            help="Give more output. Option is additive, "
            "and can be used up to 2 times.",
        )
        parser.add_argument(
            "-q",
            "--quiet",
            action="store_true",
            help="Do not write the executed commands " "to standard output",
        )

        parser.parse_args = functools.partial(parser.parse_args, namespace=ns)
        return parser

    @classmethod
    def run_task(cls, task: str, args=None):
        """
        Execute a task

        Args:
            task: task name to execute
            args: list of CLI arguments, default is `sys.argv`

        Return:
            Number of failed jobs
        """
        task_config = cls.TASKS_DESCRIPTION[task]
        parser = cls.task_cli(task)
        options = parser.parse_args(args=args)
        if options.verbose == 0:
            level = logging.WARN
        elif options.verbose == 1:
            level = logging.INFO
        else:
            level = logging.DEBUG
        logging.basicConfig(level=level, format="%(levelname)s: %(message)s")
        Tool.LOG_JOBS = not options.quiet

        project = BBPProject.from_config_files()
        num_errors = 0
        if task_config.on_codebase:
            options.languages = map(str.strip, options.languages.split(","))
            options.languages = map(str.lower, options.languages)
            options.languages = list(options.languages)

            num_errors = project.run_task_on_codebase(task, **vars(options))
        else:
            num_errors = project.run_global_task(**vars(options))
        if num_errors != 0:
            logging.error("%i jobs failed", num_errors)
        return num_errors

    def run_global_task(self, task, **kwargs):
        """
        Execute a task that do not take files from codebase in argument

        Args:
            task: task name to execute
            kwargs: CLI arguments

        Return:
            Number of failed jobs
        """
        tools = list(self.tools_for_task(task, kwargs["languages"]))
        for tool in tools:
            tool.configure()
        [tool.configure() for tool in tools]
        [tool.prepare_config() for tool in tools if "config_file" in tool.config]
        num_errors = 0
        for tool in tools:
            num_errors += tool.run(task, **kwargs)
        return num_errors

    def run_task_on_codebase(self, task: str, languages=None, sources=None, **kwargs):
        """
        Execute a task working on files of the codebase

        Args:
            task: task name to execute
            languages: list of desired languages to apply the task
            sources: subset of files or dirs to apply the task
            kwargs: CLI arguments

        Return:
            Number of failed jobs
        """
        tools = list(self.tools_for_task(task, languages))
        [tool.configure() for tool in tools]
        [tool.prepare_config() for tool in tools if "config_file" in tool.config]

        if not tools:
            logging.warning(
                "No tool enabled for task %s. "
                "Consider editing file %s"
                " at the root of your project",
                task,
                self.USER_CONFIG_FILE,
            )
            return 0

        src_dirs = []
        src_others = []
        for src in sources or []:
            if os.path.isdir(src):
                src_dirs.append(src)
            else:
                src_others.append(src)

        if not sources:
            git_ls_tree_required = True
        elif src_dirs:
            git_ls_tree_required = not functools.reduce(
                operator.and_,
                (tool.config["capabilities"].cli_accept_dir for tool in tools),
            )
        else:
            git_ls_tree_required = False

        src_others = [os.path.join(os.getcwd(), f) for f in src_others]

        if git_ls_tree_required:
            cmd = [
                which("git"),
                "ls-tree",
                "-r",
                "-z",
                "--name-only",
                "--full-name",
                "HEAD",
            ]
            cmd += src_dirs
            log_command(cmd)
            if not sources or src_dirs:
                git_ls_tree = subprocess.check_output(cmd).decode("utf-8").split("\0")
        else:
            git_ls_tree = []

        num_errors = 0
        for tool in tools:
            accept_dir = tool.config["capabilities"].cli_accept_dir
            files = copy.copy(src_others)
            if not sources or (src_dirs and not accept_dir):
                files += git_ls_tree

            tasks = collections.defaultdict(set)
            for file in files:
                # build the task list per tool
                for tool in tools:
                    if tool.accepts_file(file):
                        tasks[tool].add(file)
        for tool, tool_tasks in tasks.items():
            # perform the tasks
            num_errors += tool.run(task, *tool_tasks, cwd=source_dir(), **kwargs)
        return num_errors

    def tools_for_task(self, task: str, languages):
        """
        Get the tools able to process a task on given languages

            >>> list(BBPProject.tools_for_task("format", ["C++"]))
            [ClangFormat]
            >>> list(BBPProject.tools_for_task("format", ["C++", "CMake"]))
            [ClangFormat, CMake]

        Args:
            task: task name to execute
            languages: list of languages

        Return:
            Generator of `Tool` instances
        """
        if languages is None:
            languages = BBPProject.supported_languages(task)
        for tool in self.tools.values():
            task_config = tool.config.get("provides", {}).get(task, {})
            if task_config:
                for lang in task_config["languages"]:
                    if lang.lower() in languages:
                        yield tool

    @classmethod
    def supported_languages(cls, task: str):
        """
        Get the list of languages supported by a given task

        Args:
            task: task name to execute

        Return:
            list of programming languages supported by the provider
        """
        languages = set()
        for tool_conf in cls.TOOLS_DESCRIPTION.values():
            provider_conf = tool_conf.get("provides", {}).get(task)
            if provider_conf:
                provider_langs = provider_conf.get("languages")
                assert isinstance(provider_langs, list)
                for provider_lang in provider_langs:
                    languages.add(provider_lang)
        return languages

    def __init__(self, config):
        self._config = config

    @cached_property
    def tools(self):
        tools = {}
        for name, tool_desc in BBPProject.TOOLS_DESCRIPTION.items():
            if name in self._config["tools"]:
                tools[name] = tool_desc["cls"](tool_desc, self._config["tools"][name])
        return tools

    @classmethod
    def default_config_file(cls):
        """
        Return:
            Path to the default YAML configuration file
        """
        return THIS_SCRIPT_DIR.parent.joinpath(cls.CONFIG_FILE)

    @classmethod
    def user_config_file(cls):
        """
        Locate the user configuration file in project root

        Return:
            Path to the file if found, `None` otherwise
        """
        expected_location = source_dir().joinpath(cls.USER_CONFIG_FILE)
        if expected_location.exists():
            return expected_location

    @classmethod
    def merge_user_config(cls, conf: dict, user_conf: dict):
        cls._merge_user_config_global(conf, user_conf)
        cls._merge_user_config_tools(conf, user_conf)

    @classmethod
    def _apply_global_conf(cls, conf: dict):
        global_conf = conf["tools"].pop("global", None)
        if not global_conf:
            return
        for tool in conf["tools"]:
            if tool == "global":
                continue
            tool_conf = copy.copy(global_conf)
            tool_conf.update(conf["tools"][tool])
            conf["tools"][tool] = tool_conf

    @classmethod
    def _parse_conf_regex(cls, conf: dict):
        for tool, tool_conf in conf["tools"].items():
            if tool == "global":
                continue

            def apply_on_section(section):
                include = tool_conf.get(section)
                if include is not None and include.get("match"):
                    tool_conf[section]["match"] = list(
                        map(re.compile, include["match"])
                    )

            apply_on_section("include")
            apply_on_section("exclude")

    @classmethod
    def _merge_user_config_tools(cls, conf: dict, user_conf: dict):
        tools = user_conf.get("tools", {})
        assert isinstance(tools, dict)
        for name, config in tools.items():
            enable = config.get("enable", True)
            conf["tools"][name]["enable"] = enable

    @classmethod
    def _merge_user_config_global(cls, conf: dict, user_conf: dict, path=None):
        path = [] if path is None else path
        for key in user_conf:
            if key in conf:
                # pylint: disable=C0123
                if isinstance(conf[key], dict) and isinstance(user_conf[key], dict):
                    cls._merge_user_config_global(
                        conf[key], user_conf[key], path + [str(key)]
                    )
                elif conf[key] == user_conf[key]:
                    pass
                elif type(conf[key]) == type(user_conf[key]):  # noqa: E721
                    conf[key] = user_conf[key]
                else:
                    raise Exception(f"Conflict at {'.'.join(path)}")
            else:
                conf[key] = user_conf[key]

    @classmethod
    def _exclude_disabled_tools(cls, conf: dict):
        tools = conf["tools"]
        tools = {name: tools[name] for name in tools if tools[name]["enable"]}
        conf["tools"] = tools

    @classmethod
    def _sanitize_config(cls, conf):
        tools = conf.setdefault("tools", {})
        for name in tools:
            if tools[name] is None:
                tools[name] = {}
            config = tools[name]

            def fix_re_pattern_sections(section):
                include = config.get(section)
                if include is not None and include.get("match"):
                    if isinstance(include["match"], str):
                        include["match"] = [include["match"]]

            fix_re_pattern_sections("include")
            fix_re_pattern_sections("exclude")

    @classmethod
    def from_config_files(cls):
        """
        Construct a BBPProject from:
        - the YAML file bbp-project.yaml provided by this project
        - the YAML file provided by the parent project if available.

        Return:
            Instance of `BBPProject`
        """
        cls.virtualenv().ensure_requirement("PyYAML>=5")
        import yaml  # pylint: disable=C0415

        with open(cls.default_config_file(), encoding="utf-8") as file:
            conf = yaml.safe_load(file)
            cls._sanitize_config(conf)
        for config in conf["tools"].values():
            config.setdefault("enable", True)
        user_conf_file = cls.user_config_file()
        if user_conf_file:
            with open(user_conf_file, encoding="utf-8") as file:
                user_conf = yaml.safe_load(file)
                cls._sanitize_config(user_conf)
            cls.merge_user_config(conf, user_conf)
        cls._apply_global_conf(conf)
        cls._parse_conf_regex(conf)
        cls._exclude_disabled_tools(conf)
        return BBPProject(conf)
