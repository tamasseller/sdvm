#ifndef JIT_REGISTERALLOCATOR_H_
#define JIT_REGISTERALLOCATOR_H_

#include "Immediate.h"
#include "Assembler.h"

#include "algorithm/Math.h"

class RegisterAllocator
{
public:
	enum class ValueStatus
	{
		Empty,		// There is no value associated with this slot.
		Unloaded, 	// The value resides in the associated memory location.
		Clean,    	// The value is loaded into the corresponding register.
		Dirty,    	// The corresponding register contains a new value that is not yet written out into memory.
		Copy, 		// There is a pending move into the virtual stack slot, which is not realized in any way yet.
		Immediate,  // An immediate value has been assigned to the virtual stack slot, which is not realized in any way yet.
	};

private:
	struct ValuePlacement
	{
		ValueStatus valueStatus;
		uint32_t deferredParam;
	};

	ValuePlacement topEight[8];
	const uint16_t nArgs, nRegArgs;
	uint16_t stackDepth, maxStored = 0;
	uint8_t usedCsRegs = 0;

	uint32_t memoryOffset(uint16_t idx) const;

	static ArmV6::LoReg correspondingRegister(uint16_t idx) ;

	static void summonImmediate(Assembler& a, ArmV6::LoReg target, uint32_t value);
	void loadCopy(Assembler& a, ArmV6::LoReg reg, uint16_t idx);
	void store(Assembler& a, uint16_t idx);
	void eliminateCopies(Assembler &a, uint16_t idx);

	ArmV6::LoReg allocate(Assembler& a, ValueStatus status, uint32_t param);

public:
	RegisterAllocator(uint32_t nArgs);

	inline ArmV6::LoReg acquire(Assembler& a) {
		return allocate(a, ValueStatus::Dirty, 0);
	}

	inline void pushImmediate(Assembler& a, uint32_t param) {
		allocate(a, ValueStatus::Immediate, param);
	}

	ArmV6::LoReg consume(Assembler& a);

	void pull(Assembler& a, uint32_t idx);
	void shove(Assembler& a, uint32_t idx);
	void drop(Assembler& a, uint32_t n);
};

#endif /* JIT_REGISTERALLOCATOR_H_ */
