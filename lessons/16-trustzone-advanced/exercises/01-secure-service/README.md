**Language:** English | [Türkçe](README.tr.md)

# Exercise — Scalar Secure API policy

Complete only the `policy.c` TODO: accept the closed interval [0,15].
All other uint32_t inputs must be rejected. Do not change SAU, MPC, SG, startup, or
test expectations. The starter is fail-closed: only CHECK_SERVICE=FAIL;
checks for invalid inputs and Secure memory violations remain PASS.

The `16-exercise-policy-nonsecure` target also builds its own Secure dependency.
Load the two exercise ELFs together or run `python tests/development-smoke.py 16`.
Once solved, all CHECK fields should PASS. 0,7,15 are accepted;
16 and 0xFFFFFFFF are rejected. The reference solution is in a separate directory.
