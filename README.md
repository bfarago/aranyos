# aranyos
AR.any.OS

This is only a sandbox actually. Tested on TRK-USB-MPC5643L , codewarrior compiler suit.

Atomic Increment/decrement functions to eliminate the Critical Sections.
Critical sections could have a huge overhead in execution time.
PPC cores have reservation mechanishm. PPC e200z (zen) cores have implemented it
inside one core. Multicore scenario could also be possible, when using a SEMA4 gate to
eliminate two cores interfere each-other.

A minimal OS implementation is also started here from scratch.
AranyOS was my first ARM7TDMI os, therefore I used the same name.
Later, I will merge the two project. (I hope.)
This one have a HW clock/timer, scheduler. It have not a pre-emptive kernel yet, but
this will be the next step by plan...

