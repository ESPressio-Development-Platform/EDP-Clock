# Internal API

Internal machinery implements exact count-to-nanosecond conversion, observation estimation, integer-only frequency/phase discipline, uncertainty propagation, state transitions and coherent publication through Platform atomic/snapshot facilities. These mechanisms are implementation details; consumers should depend on the public clock contracts rather than the estimator representation.
