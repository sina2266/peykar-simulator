FROM chaste/release@sha256:1041d9ac3d7eb6f3ec187bdb5aac2d9c28c5f121ff03315501baa20139141848
USER root
# Official home is 0750, preventing a caller's non-root UID from running tests.
# Grant traversal only; retain the original executables and model files.
RUN chmod o+x /home/chaste
USER chaste
