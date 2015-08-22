# claw-vm
![claw logo](http://s.lowendshare.com/7/1440283862.613.clawchip_huge%20[111240].png)

Virtual Machine that runs the CLAW bytecode. Write once, run... on the microcat.

Not all CLAW instructions are implemented yet.

Meaning of the runtime errors:

1 - Arithmetic exception (divide by zero, ...)

2 - Stack overflow

3 - Stack underflow

5 - Target out of bounds (check all jumps. The program must end with END or equivalent conditional end instruction).
