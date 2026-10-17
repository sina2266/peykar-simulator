import ctypes
import platform
from _ctypes import byref
from importlib.resources import files
import os
import sys

import pyvcell_odesolver
from pyvcell_odesolver._internal import check_arch


class VCellNativeLibraryLoader:
    lib: ctypes.CDLL

    def __init__(self) -> None:
        self.lib = self._load_library()
        self._define_entry_points()

    def _load_library(self) -> ctypes.CDLL:
        system = platform.system()
        lib_ext = {"Linux": ".so", "Darwin": ".dylib", "Windows": ".dll"}.get(system)
        version_identifier = "VCell ODE solver (CVODE/IDA) v"

        if lib_ext is None:
            raise OSError(f"Unsupported operating system: {system}")

        libs_dir = files(pyvcell_odesolver).joinpath("lib")
        if not libs_dir.is_dir():
            raise OSError(f"Could not find the shared library directory {libs_dir}")


        valid_libraries = check_arch.get_all_valid_libraries_from_dir(str(libs_dir))
        for file in valid_libraries:
            file_str = str(file)
            try:
                lib = ctypes.CDLL(name=file_str)
                lib.version_ctypes.restype = ctypes.c_char_p # signals ctypes to return as char-array
                version_str: str = lib.version_ctypes().decode("utf-8")
                if version_identifier not in version_str:
                    err_str = f"`{file_str}` isn't desired lib (`{version_identifier}` not in `{version_str}`)"
                    print(err_str, file=sys.stderr)
            except AttributeError as e:
                print(f"library `{file_str}` could not be loaded: {e}", file=sys.stderr)
                continue # If we didn't get a version, we didn't get the correct lib
            return lib

        # Didn't find what we needed
        files_checked = "\n  - ".join(valid_libraries if len(valid_libraries) > 0 else os.listdir(str(libs_dir)))
        raise OSError(f"Could not find the shared library; in `{str(libs_dir)}` checked: \n  - {files_checked}")

    def _define_entry_points(self) -> None:
        self.lib.version_ctypes.restype = ctypes.c_char_p # technically, we did this above, but just in case
        self.lib.version_ctypes.argtypes = []

        self.lib.solve_ctypes.restype = ctypes.c_int
        self.lib.solve_ctypes.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int]