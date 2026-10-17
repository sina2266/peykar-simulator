from pathlib import Path
import re
import stat
import sys
import tempfile

sys.path.append(str(Path(__file__).resolve().parent.parent))

import cpp.lib  # noqa: E402


def test_clang_tidy_conf_merger():
    orig_checks = "foo-*,bar-pika,-bar-foo"
    test_func = cpp.lib.ClangTidy.merge_clang_tidy_checks

    assert test_func(orig_checks, None) == orig_checks
    assert test_func(orig_checks, "") == orig_checks
    assert test_func(orig_checks, "-bar-pika") == "foo-*,-bar-foo,-bar-pika"
    assert test_func(orig_checks, "bar-pika") == "foo-*,-bar-foo,bar-pika"
    assert test_func(orig_checks, "-bar-*") == "foo-*,-bar-*"
    assert test_func(orig_checks, "bar-*") == "foo-*,bar-*"
    assert test_func(orig_checks, "-bar-*") == "foo-*,-bar-*"
    assert test_func(orig_checks, "-bar-foo") == "foo-*,bar-pika,-bar-foo"
    assert test_func(orig_checks, "bar-foo") == "foo-*,bar-pika,bar-foo"

    assert test_func("bar-foo", "-bar-*") == "-bar-*"

    assert test_func("", "-bar-pika") == "-bar-pika"
    assert test_func("", "bar-foo") == "bar-foo"
    assert test_func(None, None) is None


def test_where():
    """Test cpp.lib.where function"""
    with tempfile.TemporaryDirectory() as bin_dir:
        expected_paths = set()
        for name in [
            "clang-format",
            "clang-format-13",
            "clang-format-14",
            "clang-format-mp-13",
            "clang-format-mp-14",
            "clang-format-diff.py",
            "clang-format-mp-diff.py",
            "clang-format-14-diff.py",
            "clang-format-diff",
            "clang-format-mp-diff",
            "clang-format-14-diff",
        ]:
            Path(bin_dir, name).touch()
        for name in ["clang-format", "clang-format-13", "clang-format-mp-13"]:
            executable = Path(bin_dir, name)
            executable.chmod(executable.stat().st_mode | stat.S_IEXEC)
            expected_paths.add(str(executable))
        for name in [
            "clang-format-diff.py",
            "clang-format-mp-diff.py",
            "clang-format-14-diff.py",
            "clang-format-diff",
            "clang-format-mp-diff",
            "clang-format-14-diff",
        ]:
            executable = Path(bin_dir, name)
            executable.chmod(executable.stat().st_mode | stat.S_IEXEC)
        TOOLS = cpp.lib.BBPProject.TOOLS_DESCRIPTION
        names_regex = TOOLS["ClangFormat"]["names_regex"]
        names_exclude_regex = TOOLS["ClangFormat"]["names_exclude_regex"]
        paths = set(
            cpp.lib.where(
                "clang-format",
                regex=re.compile(names_regex),
                exclude_regex=re.compile(names_exclude_regex),
                paths=[bin_dir],
            )
        )
        assert paths == expected_paths
