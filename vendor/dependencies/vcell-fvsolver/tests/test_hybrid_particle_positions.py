"""Hybrid runs save particle positions at the PDE's output times when VCell asks for particle files.

VCell's "save particle files" option writes `incrementfile` / `listmols` commands on `<base>.smoldynOutput` into
the Smoldyn input. Smoldyn counts its own steps, so with SMOLDYN_STEP_MULTIPLIER > 1 those commands fire at the
wrong times. The solver disables them and writes `<base>_<NNN>.smoldynOutput` (`species(state) x y z` per molecule)
at each output time instead, NNN being the output's 1-based index in the log, which is how VCell reads them.

The fixture: 200 molecules of A in a 2 x 2 x 1 um box converting to the field B (k = 1/s), output every 0.05 s to
0.2 s, SMOLDYN_STEP_MULTIPLIER 2 (Smoldyn step 0.02 s, VCell's command cadence N = 5, i.e. every 0.1 s).
"""
import pathlib
import shutil

import pyvcell_fvsolver as fv

FIXTURE = pathlib.Path(__file__).parent / "fixtures" / "hybrid_save_particles"
BASE = "SimID_1000_0_"


def test_positions_are_written_at_every_output_time(tmp_path):
    for f in FIXTURE.iterdir():
        shutil.copy(f, tmp_path / f.name)
    template = (tmp_path / f"{BASE}.fvinput.in").read_text()
    fvinput = tmp_path / f"{BASE}.fvinput"
    fvinput.write_text(template.replace("@BASE_FILE_NAME@", str(tmp_path / BASE))
                       .replace("@SMOLDYN_INPUT_FILE@", str(tmp_path / f"{BASE}.smoldynInput")))
    assert fv.solve(str(fvinput), str(tmp_path / f"{BASE}.vcg"), str(tmp_path)) == 0

    times = [float(line.split()[-1]) for line in (tmp_path / f"{BASE}.log").read_text().splitlines() if line.strip()]
    assert times == [0.0, 0.05, 0.1, 0.15, 0.2]
    files = sorted(tmp_path.glob(f"{BASE}_[0-9][0-9][0-9].smoldynOutput"))
    assert [f.name for f in files] == [f"{BASE}_{i:03d}.smoldynOutput" for i in range(1, len(times) + 1)]
    counts = []
    for f in files:
        rows = [line.split() for line in f.read_text().splitlines()]
        assert all(len(r) == 4 and r[0] == "A(solution)" for r in rows)
        for r in rows:
            x, y, z = map(float, r[1:])
            assert 0 <= x <= 2 and 0 <= y <= 2 and 0 <= z <= 1
        counts.append(len(rows))
    assert 150 <= counts[0] <= 250  # ~200 molecules placed at t = 0
    assert counts == sorted(counts, reverse=True) and counts[-1] < counts[0]  # A only converts away
