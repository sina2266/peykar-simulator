import pyvcell_odesolver as pvco


def test_version_function():
    assert pvco.version() is not None
