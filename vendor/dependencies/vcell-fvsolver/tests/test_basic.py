import pyvcell_fvsolver as fv


def test_version_function():
    assert fv.version() is not None
