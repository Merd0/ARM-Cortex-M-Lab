**Language:** English | [Türkçe](README.tr.md)

# Reference solution — Scalar policy

`value <= 15U` accepts the entire unsigned range [0,15]. Using a signed cast or
checking equality with only one sample breaks the boundary conditions. The
`16-solution-policy` Secure/Non-secure pair passed all checks in Debug/QEMU. The policy
does not change memory attribution; a direct Secure read from NS still produces AUVIOL.
