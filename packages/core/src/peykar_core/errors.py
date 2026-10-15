"""Public domain errors; transports map them to their own error representations."""


class PeykarError(Exception):
    """A predictable platform error."""


class NotFound(PeykarError):
    """A referenced resource does not exist."""


class Conflict(PeykarError):
    """An immutable version, idempotency key, or state transition conflicts."""


class InvalidRequest(PeykarError):
    """A structurally valid request violates scientific or execution constraints."""


class OwnershipLost(Conflict):
    """A worker token has been fenced by another terminal transition."""


class Unsupported(PeykarError):
    """A requested capability has no executable implementation."""


class Cancelled(PeykarError):
    """Cooperative execution was cancelled."""
