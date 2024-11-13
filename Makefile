N = 100000000
PARAMS = $(N)
PROCESSORS = 64 # Max: 1772

CC = mpicc
SRC = qs_mpi.c
DEP = qs_mpi.h
ZIP = fulle637.tar.gz
SUBMISSION_DIR = fulle637
HOSTFILE = # -hostfile hostfile.txt

BUILD = build
EXE_FAST = $(BUILD)/qs_fast.o
EXE_DEBUG = $(BUILD)/qs_debug.o
ASM = qs.s

.PHONY: all clean dir run submission test

all: build

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
	mkdir -p $(SUBMISSION_DIR)
	pandoc readme.md -o $(SUBMISSION_DIR)/readme.pdf
	find . -maxdepth 1 -type f ! -name '.gitignore' ! -name 'hosts.txt' ! -name 'hostfile.txt' -exec cp {} $(SUBMISSION_DIR)/ \;
	tar -czvf $(ZIP) $(SUBMISSION_DIR)

dir:
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD) $(SUBMISSION_DIR) $(ZIP) readme.pdf output.txt
	git repack -a -d --depth=2500 --window=2500
	git gc --aggressive --prune=now
	git gc --aggressive --prune=now
	git gc --aggressive --prune=now
	git gc --aggressive --prune=now
