#!/usr/bin/env python3

import os
import posixpath
import subprocess
import sys

# get the directory of this script
test_dir = os.path.dirname(os.path.realpath(__file__))
# in the path replace \ with /, D:\ with /d/
test_dir = test_dir.replace("\\", "/")
# tell os.path.join to use / as the path separator
os.path.sep = "/"
exe = sys.argv[1]

print(f"test_dir: {test_dir}")
print(f"exe: {exe}")

input_file = posixpath.join(test_dir, "SimID_273069657_0_.nfsimInput")
output = posixpath.join(test_dir, "SimID_273069657_0_.gdat")
expected_output = posixpath.join(test_dir, "SimID_273069657_0_.gdat.expected")
species = posixpath.join(test_dir, "SimID_273069657_0_.species")
expected_species = posixpath.join(test_dir, "SimID_273069657_0_.species.expected")

if not posixpath.exists(exe):
    print(f"NFsim executable {exe} not found. Exiting...")
    sys.exit(1)

if not posixpath.exists(input_file):
    print(f"Input file {input_file} not found. Exiting...")
    sys.exit(1)

if not posixpath.exists(expected_output):
    print(f"Expected output file {expected_output} not found. Exiting...")
    sys.exit(1)

if not posixpath.exists(expected_species):
    print(f"Expected species file {expected_species} not found. Exiting...")
    sys.exit(1)

command = [exe, "-seed", "505790288", "-vcell", "-xml", input_file, "-o", output, "-sim", "1.0", "-ss", species, "-oStep", "20", "-notf", "-utl", "1000", "-cb", "-pcmatch", "-tid", "0"]
print(" ".join(command))

try:
    subprocess.check_call(command)
except subprocess.CalledProcessError:
    print("NFsim failed to run. Exiting...")
    sys.exit(1)

# verify that the output files exist
if not os.path.isfile(output):
    print(f"Output file {output} not found. Exiting...")
    sys.exit(1)

if not os.path.isfile(species):
    print(f"Species file {species} not found. Exiting...")
    sys.exit(1)

# verify that the output files match the expected output files
if open(output).read() != open(expected_output).read():
    print(f"Output file {output} does not match expected output {expected_output}. Exiting...")
    sys.exit(1)

if open(species).read() != open(expected_species).read():
    print(f"Species file {species} does not match expected species {expected_species}. Exiting...")
    sys.exit(1)

print("NFsim solver completed and solution matched expected output. Exiting...")
sys.exit(0)