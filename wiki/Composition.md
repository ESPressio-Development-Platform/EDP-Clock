# Composition

Clock providers declare capabilities in the Clock domain and external Platform requirements where coherent atomic/snapshot publication is needed. The architecture keeps source selection compile-time and Bootstrap-owned; no runtime clock registry is introduced.

Clock resolution is an advertised property. Sources used for strict synchronized time must satisfy the required sub-millisecond resolution contract.
