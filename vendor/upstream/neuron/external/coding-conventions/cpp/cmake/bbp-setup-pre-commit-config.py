import argparse
from collections import namedtuple
import copy
import logging
import os
import sys

import yaml

try:
    from yaml import CLoader as Loader
except ImportError:
    from yaml import Loader

HPC_PRE_COMMITS_REPO_URL = "https://github.com/BlueBrain/hpc-pre-commits"
DEFAULT_HPC_PRE_COMMIT_REPO = dict(
    repo=HPC_PRE_COMMITS_REPO_URL, rev="master", hooks=[]
)


class PreCommitConfig:
    def __init__(self, file):
        self._file = file
        if os.path.exists(file):
            with open(file) as istr:
                self._config = yaml.load(istr, Loader=Loader) or {}
        else:
            self._config = {}
        self._bbp_repo = self._initialize_bbp_repo()
        self._previous_config = copy.deepcopy(self._config)

    def _initialize_bbp_repo(self):
        repos = self.config.setdefault("repos", [])
        for repo in repos:
            if repo["repo"] == HPC_PRE_COMMITS_REPO_URL:
                for k, v in DEFAULT_HPC_PRE_COMMIT_REPO.items():
                    repo.setdefault(k, v)
                return repo
        bbp_repo = DEFAULT_HPC_PRE_COMMIT_REPO
        repos.append(bbp_repo)
        return bbp_repo

    @property
    def config(self):
        return self._config

    def _enable_hook(self, new_hook):
        logging.info(f"Enable hook {new_hook['name']}")
        for hook in self._bbp_repo["hooks"]:
            if (hook["id"], hook.get("name")) == (new_hook["id"], new_hook.get("name")):
                hook.update(**new_hook)
                break
        else:
            self._bbp_repo["hooks"].append(new_hook)

    def enable_cmake_hook(self, name, stages, args, extra=None):
        config = dict(
            id="hpc-pc-cmake-build",
            args=args,
            name=name,
            stages=stages,
        )
        if extra:
            config.update(extra)
        self._enable_hook(config)

    def enable_cmake_target_hook(self, stages, build_dir, target, **kwargs):
        self.enable_cmake_hook(
            target, stages, ["--build", build_dir, "--target", target], **kwargs
        )

    def enable_cmake_script_hook(self, name, stages, file, **kwargs):
        self.enable_cmake_hook(name, stages, ["-P", file], **kwargs)

    def disable_cmake_hook(self, name):
        for i, hook in enumerate(self._bbp_repo["hooks"]):
            if (name, "hpc-pc-cmake-build") == (hook.get("name"), hook["id"]):
                self._bbp_repo["hooks"].pop(i)
                break

    def save(self):
        if self._previous_config != self.config:
            logging.info("Updating pre-commit config file: %s", self._file)
            with open(self._file, "w") as ostr:
                yaml.dump(self.config, ostr, default_flow_style=False)
        else:
            logging.info("pre-commit config is up to date: %s", self._file)


class CMakeTargetHook(namedtuple("CMakeTargetHook", ["build_dir", "target"])):
    def enable(self, config, stages):
        config.enable_cmake_target_hook(
            stages, self.build_dir, self.target, extra=dict(always_run=True)
        )

    def disable(self, config):
        config.disable_cmake_hook(self.target)

    @property
    def name(self):
        return self.target


class CMakeScriptHook(namedtuple("CMakeScriptHook", ["name", "file"])):
    def enable(self, config, stages):
        config.enable_cmake_script_hook(
            self.name, stages, self.file, extra=dict(always_run=True, verbose=True)
        )

    def disable(self, config):
        config.disable_cmake_hook(self.name)


def _parse_cli(args=None):
    parser = argparse.ArgumentParser(description="Ensure CMake files formatting")
    parser.add_argument(
        "--commit-checks",
        help="Comma-separated list of checks to perform when committing changes",
        default="",
    )
    parser.add_argument(
        "--push-checks",
        help="Comma-separated list of checks to perform when pushing changes",
        default="",
    )
    parser.add_argument("source_dir", help="CMake source directory")
    parser.add_argument("build_dir", help="CMake binary directory")
    return parser.parse_args(args=args)


def fix_hook_file(hook):
    if not os.path.exists(hook):
        return
    with open(hook) as istr:
        lines = istr.readlines()
    shebang = f"#!/usr/bin/env {sys.executable}\n"
    if lines[0] == "#!/usr/bin/env python\n":
        logging.warning(f"Patching git hook script: {hook}")
        lines[0] = shebang
        with open(hook, "w") as ostr:
            ostr.writelines(lines)


def main(**kwargs):
    args = _parse_cli(**kwargs)
    PRE_COMMIT_CONFIG = os.path.join(args.source_dir, ".pre-commit-config.yaml")
    ALL_HOOKS = [
        CMakeTargetHook(args.build_dir, "check-clang-format"),
        CMakeTargetHook(args.build_dir, "check-cmake-format"),
        CMakeTargetHook(args.build_dir, "clang-tidy"),
        CMakeScriptHook(
            "courtesy-msg", os.path.join(args.build_dir, "git-push-message.cmake")
        ),
    ]

    config = PreCommitConfig(PRE_COMMIT_CONFIG)

    hooks = dict((hook.name, hook) for hook in ALL_HOOKS)
    assert len(hooks) == len(ALL_HOOKS), "hooks must have unique names"

    # Build a dictionary from the two CLI arguments
    # For example: {"clang-tidy": ["commit"], "courtesy-msg": ["commit", "push"]}
    hook_stages = {}
    for names, stage in [(args.commit_checks, "commit"), (args.push_checks, "push")]:
        for name in names.split(","):
            name = name.strip()
            if name:
                hook_stages.setdefault(name, []).append(stage)

    # Enable hooks mentioned in `hook_stages`
    for name, stages in hook_stages.items():
        hook = hooks.pop(name, None)
        if hook is None:
            logging.warning(
                f"Unknown check named: '{name}', "
                f"available checks: {', '.join(hooks.keys())}"
            )
            continue
        hook.enable(config, stages)

    # Disable the other ones
    for hook in hooks.values():
        hook.disable(config)

    config.save()

    fix_hook_file(os.path.join(args.source_dir, ".git", "hooks", "pre-commit"))
    fix_hook_file(os.path.join(args.source_dir, ".git", "hooks", "pre-push"))


if __name__ == "__main__":
    level = logging.INFO if "VERBOSE" in os.environ else logging.WARN
    logging.basicConfig(level=level, format="%(message)s")
    main()
