import subprocess
import sys

reqs = subprocess.check_output([sys.executable, '-m', 'pip', 'list'])

output_lines = reqs.splitlines()
# Remove two header lines from pip list output.
output_lines = output_lines[2:]

installed_packages = [r.decode().split()[0] for r in output_lines]

check_package = sys.argv[1]
result = 0 if check_package in installed_packages else 1

sys.exit(result)
