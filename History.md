PLANNING
========

design goals
------------

 - stable platform for reusable MCU applications
 - ability to use modern managed source languages (dart mainly, but should be pretty straight forward to add others)
 - support for async/await and stream based co-operative multitasking (but no threading)
 - robust, full featured debuging without specialized hardware
 - usable performance for the 97% of code where it does not matter that much (not number crunching oriented)
 - interoperability with HW mechanism (peripheral and raw memory buffer access, interrupt handling mechanism)

design forces
-------------

1. compatibility
 - java-like memory scheme
 - garbage collection

2. limited memory space
 - fragmentation
 - relatively few objects
 - no space for JIT

3. limited code space 
 - small, simple runtime
 - compact program representation

PROTOTYPING
===========

throwaway demonstrator
----------------------

 - purpose: to make design choices
 - has similar structure to the actual thing but uses every possible shortcut 
 - incrementaly becomes more and more complete and similar to the real deal
 - generator <-> decoder <-> executor <-> object storage separation to enable independent experiments

design choices
--------------

1. memory model
 - object storage ("heap")
 - runtime type information
 - object traceability (marking during gc)

2. program anatomy
 - type definitions
 - code in methods/functions/procedures with additional lookup tables for gc & exc

3. execution model
 - bytecode structure
   - decoding&execution efficiency vs compactness -> implementation strategy: interpret/semi-JIT/JIT/AOT
   - easy jumparound for local control flow (byte aligned code)
 - execution granularity
   - breakpoint at any bytcode isn
   - no preemption
   - GC possible only at predictable points
 - invocation mechanism
   - locals & opstack (stack frames)
   - co-routine support (suspendability for async/await)
   - opstack traceability (imprecise vs runtime val/ref tracking vs pre-cooked table)

implementation notes
--------------------

 - flow: source code -> compiler -> bytecode   ->   vm
                        |                           ^
                        \-> debug info -> debugger -/
 - multiple transformation steps with gradual desugaring -> information filtering, extraction and structuring
 - strategy: first build all layers with minimal functionality and test thoroughly, then add the more complicated features
 - initially a simplified skeleton (no exceptions, no arrays, no virtual calls, no non-32bit types, no debug info, no optimization)
 - bottom-up then top-down iterations until they converge
 - go only up and down so far as it is needed for the experiment
 - cleanliness over efficiency
 
experience
----------

 - the interpreter itself is easy
 - implementing proper bytecode encoding can be postponed
 - compiler does many things so it can not be simplified to triviality, because
   - the input (source) and output (bytecode) are very different by nature
   - it practically must be separated into a series of conversions (e.g. source -> AST -> IR -> bytecode)
   - if conversions are kept reasonably generic and simple, they tend to introduce additional junk in their outputs.
   - intra-layer transformation steps ("optimizations") can eliminate some of the inefficiency intrduced by the simplistic intermediate converters.
   - at least the most common optimization techniques must be applied in order to get a reasonably clean output in a systematic manner.
   - even though the main purpose is to iron out issues of intermediate conversions, these extra steps also improve badly written programs somewhat.
   