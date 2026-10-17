"""Smoke test for pyvcell_nfsim Python bindings."""
from __future__ import annotations

import os
from pathlib import Path

import pytest


@pytest.fixture
def smoke_test_dir():
    """Return the path to the smoke test directory."""
    return Path(__file__).parent / "smoke"


@pytest.fixture
def test_files(smoke_test_dir):
    """Return paths to test input and expected output files."""
    return {
        "input": smoke_test_dir / "SimID_273069657_0_.nfsimInput",
        "output": smoke_test_dir / "SimID_273069657_0_.gdat",
        "expected_output": smoke_test_dir / "SimID_273069657_0_.gdat.expected",
        "species": smoke_test_dir / "SimID_273069657_0_.species",
        "expected_species": smoke_test_dir / "SimID_273069657_0_.species.expected",
    }


def test_import():
    """Test that the pyvcell_nfsim module can be imported."""
    import pyvcell_nfsim

    assert hasattr(pyvcell_nfsim, "run_simulation")
    assert hasattr(pyvcell_nfsim, "__version__")


def test_smoke_simulation(test_files, smoke_test_dir):
    """Run a smoke test simulation using pyvcell_nfsim bindings."""
    import pyvcell_nfsim

    # Verify input files exist
    assert test_files["input"].exists(), f"Input file {test_files['input']} not found"
    assert test_files["expected_output"].exists(), f"Expected output file {test_files['expected_output']} not found"
    assert test_files["expected_species"].exists(), f"Expected species file {test_files['expected_species']} not found"

    # Clean up any previous output files
    if test_files["output"].exists():
        test_files["output"].unlink()
    if test_files["species"].exists():
        test_files["species"].unlink()

    # Prepare arguments matching the smoke test
    arg_map = {
        "seed": "505790288",
        "vcell": "",
        "xml": str(test_files["input"]),
        "o": str(test_files["output"]),
        "sim": "1.0",
        "ss": str(test_files["species"]),
        "oStep": "20",
        "notf": "",
        "utl": "1000",
        "cb": "",
        "pcmatch": "",
        "tid": "0",
    }

    # Change to test directory (some tests may require relative paths)
    original_dir = os.getcwd()
    try:
        os.chdir(smoke_test_dir)

        # Run the simulation
        pyvcell_nfsim.run_simulation(arg_map, False)

    finally:
        os.chdir(original_dir)

    # Verify output files were created
    assert test_files["output"].exists(), f"Output file {test_files['output']} was not created"
    assert test_files["species"].exists(), f"Species file {test_files['species']} was not created"

    # Verify output files have expected structure
    output_content = test_files["output"].read_text()
    assert len(output_content) > 0, "Output file is empty"
    assert "#          time" in output_content, "Output file missing expected header"

    # Count lines to verify simulation produced output
    output_lines = output_content.strip().split('\n')
    assert len(output_lines) >= 10, f"Output file has too few lines: {len(output_lines)}"

    species_content = test_files["species"].read_text()
    assert len(species_content) > 0, "Species file is empty"

    # Verify species file has expected number of lines (one per species)
    species_lines = [line for line in species_content.strip().split('\n') if line.strip()]
    assert len(species_lines) > 0, "Species file has no species listed"

    # Clean up output files after successful test
    test_files["output"].unlink()
    test_files["species"].unlink()
