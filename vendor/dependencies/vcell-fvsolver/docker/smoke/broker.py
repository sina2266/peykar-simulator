#!/usr/bin/env python3
"""A stand-in for the VCell broker's REST port: record every POST the solver's messaging makes.

    broker.py <port> <logfile>

Each request line is appended to <logfile> as one JSON object ({"path": ..., "query": {...}}), so a
test can check that the WorkerEvent status messages (JOB_STARTING 999, JOB_PROGRESS 1001,
JOB_COMPLETED 1003, ...) arrived. Answers 200 to everything.
"""
import json
import sys
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import parse_qs, urlsplit


class Handler(BaseHTTPRequestHandler):
    def do_POST(self) -> None:  # noqa: N802
        url = urlsplit(self.path)
        rec = {"path": url.path, "query": {k: v[-1] for k, v in parse_qs(url.query).items()}}
        with open(sys.argv[2], "a") as f:
            f.write(json.dumps(rec) + "\n")
        length = int(self.headers.get("Content-Length") or 0)
        if length:
            self.rfile.read(length)
        self.send_response(200)
        self.end_headers()

    do_GET = do_POST

    def log_message(self, *args) -> None:
        pass


if __name__ == "__main__":
    ThreadingHTTPServer(("0.0.0.0", int(sys.argv[1])), Handler).serve_forever()
