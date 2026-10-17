"""Write the smoke-test input into a work directory.

    python smoke/prepare.py <workdir> [--output-dir DIR] [--broker HOST:PORT]

Copies smoke/SimID_254696951_0_mb.xml (a VCell-generated MovingBoundary input:
MBswept, a circle translating with velocity (sin t, cos t), two diffusing
species, 31x31 nodes, t in [0, 1]) to <workdir>, pointing its outputFilePrefix
at DIR (default /simdata, the container path SlurmProxy binds). With --broker,
also adds the <jms> block VCell's MovingBoundaryFileWriter writes for HPC runs,
so a run with `-tid <n>` reports its worker events to HOST:PORT.
"""

from __future__ import annotations

import argparse
import re
from pathlib import Path

HERE = Path(__file__).resolve().parent
NAME = "SimID_254696951_0_mb.xml"

JMS = """  <jms>
    <broker>{broker}</broker>
    <jmsUser>clientUser</jmsUser>
    <pw>dummy</pw>
    <queue>workevent</queue>
    <topic>servicecontrol</topic>
    <vcellUser>smoketest</vcellUser>
    <simKey>254696951</simKey>
    <jobIndex>0</jobIndex>
  </jms>
"""


def main() -> None:
    p = argparse.ArgumentParser()
    p.add_argument("workdir")
    p.add_argument("--output-dir", default="/simdata")
    p.add_argument("--broker")
    args = p.parse_args()
    xml = (HERE / NAME).read_text()
    out_dir = args.output_dir.rstrip("/\\")
    xml = re.sub(
        r"<outputFilePrefix>.*</outputFilePrefix>",
        lambda _: f"<outputFilePrefix>{out_dir}/SimID_254696951_0_</outputFilePrefix>",
        xml,
    )
    if args.broker:
        xml = xml.replace("</MovingBoundarySetup>", JMS.format(broker=args.broker) + "</MovingBoundarySetup>")
    work = Path(args.workdir)
    work.mkdir(parents=True, exist_ok=True)
    (work / NAME).write_text(xml)
    print(f"wrote {work / NAME} (output prefix {out_dir}/SimID_254696951_0_, broker {args.broker})")


if __name__ == "__main__":
    main()
