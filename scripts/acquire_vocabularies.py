"""Explicitly acquire the reviewed vocabulary catalog as data, checking every byte.

Existing source files are verified, never overwritten. Changed provider responses
fail the known digest; they require a newly reviewed snapshot, not a silent update.
"""

import argparse
import hashlib
import json
import os
import tempfile
from pathlib import Path
from urllib.parse import urlparse
from urllib.request import Request, urlopen

ROOT = Path(__file__).resolve().parents[1]


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--catalog", type=Path, default=ROOT / "biological_libraries/vocabularies/sources.json"
    )
    parser.add_argument("--destination", type=Path, default=ROOT / ".peykar/external/vocabularies")
    args = parser.parse_args()
    args.destination.mkdir(parents=True, exist_ok=True)
    for record in json.loads(args.catalog.read_bytes()):
        name = record["file"]
        if Path(name).name != name or name.startswith("."):
            raise ValueError("Source files require simple nonhidden names")
        target = args.destination / name
        if target.is_symlink():
            raise ValueError("Vocabulary destination cannot be a symlink")
        expected = record["spec"]["raw_digest"]
        if target.exists():
            if hashlib.sha256(target.read_bytes()).hexdigest() != expected:
                raise ValueError("Existing vocabulary source differs: " + name)
        else:
            url = record["spec"]["source_url"]
            if urlparse(url).scheme != "https":
                raise ValueError("Vocabulary acquisition requires HTTPS")
            fd, path = tempfile.mkstemp(prefix=".vocabulary-", dir=args.destination)
            try:
                size = 0
                checksum = hashlib.sha256()
                with (
                    os.fdopen(fd, "wb") as output,
                    urlopen(
                        Request(
                            url,
                            headers={
                                "User-Agent": "Peykar explicit reviewed vocabulary acquisition",
                                "Accept": "application/json"
                                if name.endswith(".json")
                                else "text/plain",
                            },
                        ),
                        timeout=60,
                    ) as response,
                ):
                    if urlparse(response.url).scheme != "https":
                        raise ValueError("HTTPS downgrade is unsupported")
                    while chunk := response.read(1024**2):
                        size += len(chunk)
                        if size > 64 * 1024**2:
                            raise ValueError("Vocabulary exceeds 64 MiB")
                        checksum.update(chunk)
                        output.write(chunk)
                    output.flush()
                    os.fsync(output.fileno())
                if checksum.hexdigest() != expected:
                    raise ValueError(
                        "Provider bytes changed; review a new immutable source snapshot: " + name
                    )
                os.link(path, target)
            finally:
                Path(path).unlink(missing_ok=True)
        print(json.dumps({"file": name, "sha256": expected, "status": "verified"}), flush=True)


if __name__ == "__main__":
    main()
