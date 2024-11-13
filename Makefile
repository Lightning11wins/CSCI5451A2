N = 100000000
PARAMS = $(N)
PROCESSORS = 16 # Max: 1772

CC = mpicc
SRC = qs_mpi.c
DEP = qs_mpi.h
ZIP = fulle637.tar.gz
HOSTFILE = # -hostfile hostfile.txt

BUILD = build
EXE_FAST = $(BUILD)/qs_fast.o
EXE_DEBUG = $(BUILD)/qs_debug.o
ASM = qs.s

.PHONY: all clean dir run submission test

run: $(EXE_FAST)
	mpirun -np $(PROCESSORS) $(HOSTFILE) ./$(EXE_FAST) $(PARAMS)

debug: $(EXE_DEBUG)
	mpirun -np $(PROCESSORS) $(HOSTFILE) ./$(EXE_DEBUG) $(PARAMS)

verify:
	gcc -O3 -Wall verify.c -o $(BUILD)/verify.o
	./$(BUILD)/verify.o

build: dir $(EXE_FAST)

submission: $(ZIP)

$(EXE_FAST): $(SRC) $(DEP)
	$(CC) -O3 -march=native -Wall $(SRC) -o $(EXE_FAST)

$(EXE_DEBUG): $(SRC) $(DEP)
	$(CC) -O0 -g -Wall $(SRC) -o $(EXE_DEBUG)

$(ZIP): clean
	pandoc readme.md -o readme.pdf
	tar --exclude='.gitignore' --exclude='$(BUILD)' -czvf $(ZIP) *

dir:
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD) $(ZIP) readme.pdf output.txt
	git repack -a -d --depth=2500 --window=2500
	git gc --aggressive --prune=now
	git gc --aggressive --prune=now
	git gc --aggressive --prune=now
	git gc --aggressive --prune=now
