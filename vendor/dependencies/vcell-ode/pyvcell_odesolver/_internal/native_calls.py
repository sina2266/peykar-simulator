import ctypes
import logging
from pathlib import Path

from pydantic import BaseModel

from pyvcell_odesolver._internal.native_utils import VCellNativeLibraryLoader


class ReturnValue(BaseModel):
    success: bool
    message: str


class MutableString:
    def __init__(self, value: str):
        self.value: str = value


class ODENativeCalls:
    def __init__(self) -> None:
        self.loader = VCellNativeLibraryLoader()
        self.lib: ctypes.CDLL = self.loader.lib

    def call_version(self) -> ReturnValue:
        try:
            version_str: str = self.lib.version_ctypes()
            if version_str is None:
                error_msg = "Failed to collect and covert version information"
                logging.error(error_msg)
                return ReturnValue(success=False, message=error_msg)
            return ReturnValue(success=True, message=version_str.decode("utf-8"))
        except Exception as e:
            logging.exception("Error in vcml_to_finite_volume_input()", exc_info=e)
            raise

    def call_solve(
            self, input_file_path: str, output_file_path: str, task_id: int
    ) -> ReturnValue:
        try:
            solver_return_code: int = self.lib.solve_ctypes(
                ctypes.c_char_p(input_file_path.encode("utf-8")),
                ctypes.c_char_p(output_file_path.encode("utf-8")),
                ctypes.c_int(task_id),
            )
            if solver_return_code == 0:
                return ReturnValue(success=True, message="Solver returned successfully.")
            error_msg = "Solver failed with exit code {}".format(solver_return_code)
            logging.error(error_msg)
            return ReturnValue(success=False, message=error_msg)
        except Exception as e:
            logging.exception("Error in calling SundialsSolverInterface::solve_ctypes(...)", exc_info=e)
            raise e
