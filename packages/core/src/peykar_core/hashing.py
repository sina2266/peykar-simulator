"""Canonical JSON hashing for the restricted Peykar JSON value domain.

This is deliberately not a claim of RFC 8785 compliance. JSON numbers must be finite.
"""

import hashlib
import json
from typing import Any

from pydantic import BaseModel


def canonical_json(value: Any) -> str:
    if isinstance(value, BaseModel):
        value = value.model_dump(mode="json")
    return json.dumps(
        value, sort_keys=True, separators=(",", ":"), ensure_ascii=False, allow_nan=False
    )


def digest(value: Any) -> str:
    return hashlib.sha256(canonical_json(value).encode("utf-8")).hexdigest()


def bytes_digest(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()
