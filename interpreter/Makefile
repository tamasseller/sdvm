all: interpret.elf

run: interpret.elf
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

%.cpp.o: %.cpp
	arm-none-eabi-g++ $(CXXFLAGS) -c -o $@ $^

%.s.o: %.s
	arm-none-eabi-as -mcpu=cortex-m0 -o $@ $^

interpret.elf: vectors.s.o interpret.s.o main.cpp.o
	arm-none-eabi-g++ $(LDFLAGS) -T memmap -o $@ $^
	