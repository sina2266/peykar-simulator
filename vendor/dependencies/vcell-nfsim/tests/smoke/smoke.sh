#!/bin/bash
set -e
EXE=$1

echo "Running NFsim solver with smoke.sh $EXE"

# get the directory of this script
TEST_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

INPUT="${TEST_DIR}/SimID_273069657_0_.nfsimInput"
OUTPUT="${TEST_DIR}/SimID_273069657_0_.gdat"
EXPECTED_OUTPUT="${TEST_DIR}/SimID_273069657_0_.gdat.expected"
SPECIES="${TEST_DIR}/SimID_273069657_0_.species"
EXPECTED_SPECIES="${TEST_DIR}/SimID_273069657_0_.species.expected"

if [ ! -f "$EXE" ]; then
    echo "NFsim executable $EXE not found. Exiting..."
    exit 1
fi
if [ ! -f "$INPUT" ]; then
    echo "Input file $INPUT not found. Exiting..."
    exit 1
fi
if [ ! -f "$EXPECTED_OUTPUT" ]; then
    echo "Expected output file $EXPECTED_OUTPUT not found. Exiting..."
    exit 1
fi
if [ ! -f "$EXPECTED_SPECIES" ]; then
    echo "Expected species file $EXPECTED_SPECIES not found. Exiting..."
    exit 1
fi


command="$EXE -seed 505790288 -vcell -xml $INPUT -o $OUTPUT -sim 1.0 -ss $SPECIES -oStep 20 -notf -utl 1000 -cb -pcmatch -tid 0"
echo "$command"
if ! $command; then
    echo "NFsim failed to run. Exiting..."
    exit 1
fi

# verify that the output files exist
if [ ! -f "$OUTPUT" ]; then
    echo "Output file $OUTPUT not found. Exiting..."
    exit 1
fi
# shellcheck disable=SC2086
if [ ! -f $SPECIES ]; then
    echo "Species file $SPECIES not found. Exiting..."
    exit 1
fi

# verify that the output files match the expected output files
if ! diff "$OUTPUT" "$EXPECTED_OUTPUT"; then
    echo "Output file $OUTPUT does not match expected output $EXPECTED_OUTPUT. Exiting..."
    exit 1
fi
if ! diff "$SPECIES" "$EXPECTED_SPECIES"; then
    echo "Species file $SPECIES does not match expected species $EXPECTED_SPECIES. Exiting..."
    exit 1
fi

echo "NFsim solver completed and solution matched expected output. Exiting..."
exit 0
