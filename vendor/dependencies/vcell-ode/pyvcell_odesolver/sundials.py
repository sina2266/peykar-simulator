from pyvcell_odesolver._internal.native_calls import ODENativeCalls

def version() -> str:
    obj = ODENativeCalls()
    return obj.call_version().message

def solve(input_file_path: str, output_file_path: str, task_id: int = -1) -> int:
    obj = ODENativeCalls()
    # In python, True == 1; we want 0 == true (a.k.a.: process returned exit code 0)
    return int(not obj.call_solve(input_file_path, output_file_path, task_id).success)

if __name__ == "__main__":
    print(version())