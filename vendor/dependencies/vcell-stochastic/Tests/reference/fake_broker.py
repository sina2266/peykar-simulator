#!/usr/bin/env python3
"""A stand-in for VCell's message broker REST endpoint, for the messaging smoke test.

A messaging build of VCellStoch, given a JMS_PARAM block and a trailing ``-tid <n>``, POSTs each
worker event to ``http://<JMS_BROKER>/api/message/workerEvent?...``. This server answers 200 to
everything and appends each request line to a log, one per line, so CI can assert that a
JOB_COMPLETED event (WorkerEvent_Status=1003) arrived with the right TaskID.

    fake_broker.py <port> <log file>
"""

import sys
from http.server import BaseHTTPRequestHandler, HTTPServer


class Handler(BaseHTTPRequestHandler):
    def _answer(self) -> None:
        length = int(self.headers.get("Content-Length") or 0)
        if length:
            self.rfile.read(length)
        with open(LOG, "a") as f:
            f.write(f"{self.command} {self.path}\n")
        self.send_response(200)
        self.send_header("Content-Length", "0")
        self.end_headers()

    do_POST = do_PUT = do_GET = _answer

    def log_message(self, *args: object) -> None:
        pass


if __name__ == "__main__":
    port, LOG = int(sys.argv[1]), sys.argv[2]
    HTTPServer(("127.0.0.1", port), Handler).serve_forever()
