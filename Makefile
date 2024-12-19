N = 1000000
PARAMS = $(N) "output.txt"
PROCESSORS = 16 # Max: 1772

CC = mpicc
SRC = qs_mpi.c
DEP = qs_mpi.h
ZIP = fulle637.tar.gz
SUBMISSION_DIR = fulle637
HOSTFILE = # -hostfile hostfile.txt

BUILD = build
EXE_FAST = $(BUILD)/qs_fast.o
EXE_DEBUG = $(BUILD)/qs_debug.o
EXE = qs_mpi
ASM = qs.s

.PHONY: all clean dir run submission test

build: $(EXE)

run: $(EXE_FAST)
	mpirun -np $(PROCESSORS) $(HOSTFILE) ./$(EXE_FAST) $(PARAMS)

debug: $(EXE_DEBUG)
	mpirun -np $(PROCESSORS) $(HOSTFILE) ./$(EXE_DEBUG) $(PARAMS)

verify:
	gcc -O3 -Wall verify.c -o $(BUILD)/verify.o
	./$(BUILD)/verify.o

submission: $(ZIP)

$(EXE_FAST): $(SRC) $(DEP) dir
	$(CC) -O3 -march=native -Wall $(SRC) -o $(EXE_FAST)

$(EXE_DEBUG): $(SRC) $(DEP) dir
	$(CC) -O0 -g -Wall $(SRC) -o $(EXE_DEBUG)

$(EXE): $(SRC) $(DEP)
	$(CC) -O3 -march=native -Wall $(SRC) -o $(EXE)

dir:
	mkdir -p $(BUILD)

$(ZIP): clean
	mkdir -p $(SUBMISSION_DIR)
	pandoc readme.md -o $(SUBMISSION_DIR)/readme.pdf
	find . -maxdepth 1 -type f ! -name '.gitignore' ! -name 'hosts.txt' ! -name 'hostfile.txt' -exec cp {} $(SUBMISSION_DIR)/ \;
	tar -czvf $(ZIP) $(SUBMISSION_DIR)

clean:
	rm -rf $(BUILD) $(SUBMISSION_DIR) $(ZIP) $(EXE) readme.pdf output.txt
	git repack -a -d --depth=2500 --window=2500
	git gc --aggressive --prune=now
