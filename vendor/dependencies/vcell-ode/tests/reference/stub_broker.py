#!/usr/bin/env python3
"""A stand-in for the ActiveMQ REST endpoint the messaging build POSTs worker events to.

Logs the request line (path + query) of every request, one per line, to <log>, and answers 200.
Used by the container smoke test to assert that JOB_COMPLETED (WorkerEvent_Status=1003) reaches the
broker for a `-tid` run.

    stub_broker.py <port> <log>
"""

import sys
from http.server import BaseHTTPRequestHandler, HTTPServer


class Handler(BaseHTTPRequestHandler):
    def _log_and_ok(self) -> None:
        length = int(self.headers.get("Content-Length") or 0)
        if length:
            self.rfile.read(length)
        with open(sys.argv[2], "a") as f:
            f.write(f"{self.command} {self.path}\n")
        self.send_response(200)
        self.send_header("Content-Length", "0")
        self.end_headers()

    do_POST = _log_and_ok
    do_GET = _log_and_ok

    def log_message(self, *args: object) -> None:
        pass


HTTPServer(("127.0.0.1", int(sys.argv[1])), Handler).serve_forever()
