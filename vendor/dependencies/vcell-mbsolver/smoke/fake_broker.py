"""A stand-in for VCell's broker REST endpoint, to prove messaging is compiled in.

    python smoke/fake_broker.py <port> <log-file>     # serve (run it in the background)
    python smoke/fake_broker.py --wait <port>         # block until it accepts connections

Answers every POST with 200 and appends its path+query to <log-file>, one per
line. vcell-messaging posts each worker event (starting, progress, completed,
failure) to http://<broker>/api/message/workerEvent?...&WorkerEvent_Status=<code>&...
"""

from __future__ import annotations

import socket
import socketserver
import sys
import time
from http.server import BaseHTTPRequestHandler, HTTPServer


class Handler(BaseHTTPRequestHandler):
    def do_POST(self) -> None:  # noqa: N802
        length = int(self.headers.get("Content-Length") or 0)
        if length:
            self.rfile.read(length)
        with open(sys.argv[2], "a") as log:
            log.write(self.path + "\n")
        self.send_response(200)
        self.send_header("Content-Length", "0")
        self.end_headers()

    def log_message(self, *args: object) -> None:
        pass


class Server(HTTPServer):
    # HTTPServer.server_bind looks up the host's FQDN, which can stall for ~30 s on
    # macOS runners before the socket listens; the plain TCP bind is all this needs.
    def server_bind(self) -> None:
        socketserver.TCPServer.server_bind(self)
        self.server_name, self.server_port = "127.0.0.1", self.server_address[1]


def wait(port: int, timeout: float = 60.0) -> None:
    deadline = time.monotonic() + timeout
    while True:
        try:
            socket.create_connection(("127.0.0.1", port), timeout=1).close()
            return
        except OSError:
            if time.monotonic() > deadline:
                raise
            time.sleep(0.1)


if __name__ == "__main__":
    if sys.argv[1] == "--wait":
        wait(int(sys.argv[2]))
        sys.exit(0)
    Server(("127.0.0.1", int(sys.argv[1])), Handler).serve_forever()
