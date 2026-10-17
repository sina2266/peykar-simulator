#!/usr/bin/env python3
"""A stand-in for VCell's ActiveMQ REST endpoint, for the messaging smoke test.

    fake_broker.py <port> <log file>

Accepts every request with 200 and appends its request line (method + path, which carries the
WorkerEvent properties as the query string) to the log file, one per line.
"""

from __future__ import annotations

import sys
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

port, log_path = int(sys.argv[1]), sys.argv[2]


class Handler(BaseHTTPRequestHandler):
    def _record(self) -> None:
        length = int(self.headers.get("Content-Length") or 0)
        if length:
            self.rfile.read(length)
        with open(log_path, "a", encoding="utf-8") as log:
            log.write(f"{self.command} {self.path}\n")
        self.send_response(200)
        self.send_header("Content-Length", "0")
        self.end_headers()

    do_POST = _record
    do_GET = _record

    def log_message(self, *args: object) -> None:
        pass


ThreadingHTTPServer(("127.0.0.1", port), Handler).serve_forever()
