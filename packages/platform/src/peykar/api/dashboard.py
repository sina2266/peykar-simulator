"""Serve packaged, offline-capable client assets without exposing credentials."""

from pathlib import Path

from fastapi import FastAPI
from fastapi.responses import RedirectResponse
from fastapi.staticfiles import StaticFiles
from starlette.datastructures import MutableHeaders
from starlette.types import ASGIApp, Message, Receive, Scope, Send

CSP = (
    "default-src 'none'; script-src 'self'; style-src 'self'; "
    "connect-src 'self'; img-src 'self' data:; font-src 'self'; "
    "base-uri 'none'; object-src 'none'; frame-ancestors 'none'; form-action 'self'"
)


class WorkspaceHeaders:
    def __init__(self, app: ASGIApp) -> None:
        self.app = app

    async def __call__(self, scope: Scope, receive: Receive, send: Send) -> None:
        async def secured(message: Message) -> None:
            if message["type"] == "http.response.start":
                headers = MutableHeaders(scope=message)
                headers["X-Content-Type-Options"] = "nosniff"
                headers["Referrer-Policy"] = "no-referrer"
                headers["Cache-Control"] = "no-store"
                if scope.get("path", "").startswith("/app"):
                    headers["Content-Security-Policy"] = CSP
                    headers["X-Frame-Options"] = "DENY"
                    headers["Permissions-Policy"] = "camera=(), microphone=(), geolocation=()"
            await send(message)

        await self.app(scope, receive, secured)


def install_dashboard(app: FastAPI) -> None:
    app.add_middleware(WorkspaceHeaders)
    assets = Path(__file__).parents[1] / "web_assets"
    # Source-only development retains the API; `make dashboard` builds the assets.
    # All release packages and Docker images include the same static client.
    if (assets / "index.html").is_file():
        app.mount("/app", StaticFiles(directory=assets, html=True), name="workbench")

        @app.get("/", include_in_schema=False)
        def home() -> RedirectResponse:
            return RedirectResponse("/app/", status_code=307)
