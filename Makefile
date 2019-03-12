CC      = g++
CFLAGS  = -g
RM      = rm -f

default: all

all: microcode-swapper

Hello: microcode-swapper.cpp
	$(CC) $(CFLAGS) -o microcode-swapper microcode-swapper.cpp

clean:
	$(RM) microcode-swapper
	$(RM) *.out