all: jumpy.elf

run: jumpy.elf
	qemu-system-arm -M lm3s811evb -m 8k -nographic -s -S -kernel $<

CXXFLAGS += -O0 -g3
CXXFLAGS += -mcpu=cortex-m0
CXXFLAGS += --std=c++17
CXXFLAGS += -static-libgcc
CXXFLAGS += -fno-exceptions
CXXFLAGS += -fno-use-cxa-atexit
CXXFLAGS += -fno-rtti
CXXFLAGS += -fconcepts

LDFLAGS += -mcpu=cortex-m0
LDFLAGS += --static
LDFLAGS += -nostdlib
LDFLAGS += -nostartfiles
#LDFLAGS += -Wl,--gc-sections

jit.o : jit.cpp
	arm-none-eabi-g++ $(CXXFLAGS) -c -o $@ $^
	
%.o: %.s
	arm-none-eabi-as -mcpu=cortex-m0 -o $@ $^

jumpy.elf : vectors.o jumpy.o jit.o
	arm-none-eabi-g++ $(LDFLAGS) -T memmap -o $@ $^
	