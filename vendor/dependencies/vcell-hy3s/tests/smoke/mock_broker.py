#!/usr/bin/env python3
"""A stand-in for VCell's JMS REST bridge, for the messaging smoke test.

A messaging build started with -tid <n> reports its status by POSTing to
http://<JMS_BROKER>/api/message/workerEvent?...WorkerEvent_Status=<code>&...
(vcell-messaging, CurlProxy::sendStatus). This answers every request with 200
and appends its request line to a log, one per line, which the smoke test then
inspects for the progress (1001) and completed (1003) events.

Usage: mock_broker.py <port> <logfile>
"""
import sys
from http.server import BaseHTTPRequestHandler, HTTPServer


class Handler(BaseHTTPRequestHandler):
    def _answer(self):
        with open(sys.argv[2], "a") as log:
            log.write(f"{self.command} {self.path}\n")
        self.send_response(200)
        self.send_header("Content-Length", "0")
        self.end_headers()

    do_POST = do_GET = do_PUT = _answer

    def log_message(self, *args):
        pass


HTTPServer(("127.0.0.1", int(sys.argv[1])), Handler).serve_forever()
