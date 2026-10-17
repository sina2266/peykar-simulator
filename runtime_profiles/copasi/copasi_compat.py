"""4.47.309 SWIG output-flag bridge; no numerical/model changes.

The pinned C++ OutputFlagBase positions are 0..5. initializeRaw's pinned SWIG
extension accepts an integer bitmask while OUTPUT_UI is exposed as an opaque
CFlags pointer. Restore the public mask constants for BasiCO's documented API.
"""

import COPASI


def prepare_output_flags():
    task = COPASI.CCopasiTask
    if isinstance(task.OUTPUT_UI, int):
        return False
    version = COPASI.CVersion.VERSION.getVersion()
    if not version.startswith("4.47.309"):
        raise RuntimeError("Unreviewed COPASI output flag interface")
    names = ("INITIALIZE", "STREAM", "FINISH", "REPORT", "PLOT", "TIME_SERIES")
    positions = [getattr(task, "OutputFlagBase_" + name) for name in names]
    if positions != list(range(6)):
        raise RuntimeError("COPASI output flag layout changed")
    before, during, after, report, plot, series = [1 << p for p in positions]
    masks = {
        "NO_OUTPUT": 0,
        "OUTPUT_BEFORE": before,
        "OUTPUT_DURING": during,
        "OUTPUT_AFTER": after,
        "REPORT": report,
        "PLOT": plot,
        "TIME_SERIES": series,
        "OUTPUT": report | plot | during,
        "OUTPUT_SE": report | plot | before | during | after,
        "OUTPUT_UI": report | plot | series | before | during | after,
        "ONLY_TIME_SERIES": series | before | during | after,
    }
    for name, value in masks.items():
        setattr(task, name, value)
    return True
