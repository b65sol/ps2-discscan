EE_BIN = discscan.elf
EE_OBJS = discscan.o
EE_LIBS = -ldebug -lc -ldraw
EE_INCS = -I$(PS2SDK)/iop/include

all: $(EE_BIN)

clean:
	rm -f $(EE_BIN) $(EE_OBJS)

run: $(EE_BIN)
	ps2client execee host:$(EE_BIN)

reset:
	ps2client reset

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
