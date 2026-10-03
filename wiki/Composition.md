# Composition

Clock providers declare capabilities in the Clock domain and external Platform requirements where coherent atomic/snapshot publication is needed. The architecture keeps provider selection compile-time and Bootstrap-owned; no runtime clock registry is introduced.

Clock resolution and synchronized uncertainty limit are advertised properties. Sources used for strict synchronized time must satisfy the configured resolution/uncertainty contract.

Temporal provenance adds value types and explicit provider operations, not another Composition capability. The application/upstream coordinator owns the current Era and transition policy, then invokes the already selected `SynchronizedClockProvider`. This preserves one canonical synchronized timeline and avoids making Clock depend on Mesh, Radio, Device identity or application policy.
