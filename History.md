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
 - simple interpreter 
 - compact program representation

PROTOTYPING
===========

throwaway demonstrator
----------------------

 - purpose: to make design choices
 - has similar structure as the actual thing but uses every possible shortcut 
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
 - stack based VM with type-erased (non-validatable) bytecode
   - decoding&execution efficiency vs compactness
   - easy jumparound for local control flow (byte aligned code)
 - execution granularity
   - breakpoint at any bytcode isn
   - no preemption
   - GC possible only at predictable points
 - invocation mechanism
   - locals & opstack (stack frames)
   - co-routine support (suspendability for async/await)
   - opstack traceability (imprecise vs runtime val/ref tracking vs pre-cooked table)
