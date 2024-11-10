N = 1000000
PARAMS = $(N)
PROCESSORS = 256 # Max: 1772

CC = mpicc
SRC = qs_mpi.c
DEP = qs_mpi.h
ZIP = fulle637.tar.gz
HOSTFILE = # -hostfile hostfile.txt

BUILD = build
EXE_FAST = $(BUILD)/qs_fast.o
EXE_DEBUG = $(BUILD)/qs_debug.o
ASM = qs.s

.PHONY: all clean diff dir run submission

run: $(EXE_FAST)
	mpirun -np $(PROCESSORS) $(HOSTFILE) ./$(EXE_FAST) $(PARAMS)

build: $(EXE_FAST)

submission: $(ZIP)

$(EXE_FAST): dir $(SRC) $(DEP)
	$(CC) -O3 -march=native -Wall $(SRC) -o $(EXE_FAST)

$(EXE_DEBUG): dir $(SRC) $(DEP)
	$(CC) -g -Wall $(SRC) -o $(EXE_DEBUG)

$(ZIP): clean
	pandoc readme.md -o readme.pdf
	tar --exclude='.gitignore' --exclude='$(BUILD)' -czvf $(ZIP) *

dir:
	mkdir $(BUILD)

clean:
	rm -rf $(BUILD) $(ZIP) readme.pdf

diff:
	diff clusters.txt clusters_correct.txt > clusters_diff.txt
	diff medoids.txt medoids_correct.txt > medoids_diff.txt