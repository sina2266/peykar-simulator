.. _libCellMLCurrentThinking:

==============================
Current Thinking for libCellML
==============================

This document simply outlines some of the current rationale that has an influence on how the codebase is developed.

- Temporarily dealing with external documents that are stored on the local file system (relative to the current CellML model document).

  - Allows testing of the most common model import scenario (local files addressed with a relative URL).
  - Absolves libcellml from fetching files and communicating across the Internet.
  - Expect to provide another layer that would perform this role as a libCellML I/O library, which would allow the removal of this temporary inclusion.
  - No avenue to retrieve remote external references.

- Serialise and deserialise from a string.
- Present a useful interface not one tied to the :xml:`XML<>` serialisation structure.
- Validation is quite separate (you are free to make invalid :cellml:`CellML<>` models).
- Public API treats MathML as strings only.

  - Internal to the code generation we are currently creating our own :mathml:`MathML<>` object model based on an abstract syntax tree.
  - Minimal implementation to support the immediate requirement of code generation.
  - Expect to provide another layer that would handle MathML as a separate thing, potentially linking back to the advanced functionality envisions for symbolic analysis of the model.
  - Internal to the validator, the MathML strings are parsed into a DOM for use in schema validation against the MathML schema.
