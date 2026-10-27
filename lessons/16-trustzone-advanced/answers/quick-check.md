**Language:** English | [Türkçe](quick-check.tr.md)

# Quick Check answers

1. The IDAU can impose stricter attribution; the bus MPC can separately block an NS transaction.
2. No. NSC is a controlled entry region through SG; the Secure body and secret do not become NS.
3. SFARVALID is a separate validity bit. If clear, the register contents are not a fault address.
4. It avoids the need to validate an NS pointer's attribution, range overflow, and TOCTOU
   behavior. If a pointer API is added, these problems must be addressed separately.
5. No. Image verification/signatures, key management, and a trusted boot chain were not implemented.
