CC = gcc
CFLAGS = -g -Wall
TARGET = conversion

.DEFAULT_GOAL := help

ifeq ($(options), tz)
	META_OPS = -DTRACK_EXTREME_DT -DREMOVE_ZERO_DATA
else ifeq ($(options), t)
	META_OPS = -DTRACK_EXTREME_DT
else ifeq ($(options), z)
	META_OPS = -DREMOVE_ZERO_DATA
else
	META_OPS =
endif

%.all: $(TARGET).c
	$(CC) $(CFLAGS) $(META_OPS) -DCREATE_INT_FILE -DCREATE_VOLTAGE_FILE -o $(TARGET) $(TARGET).c
	@echo "built with CREATE_INT_FILE and CREATE_VOLTAGE_FILE"
	./$(TARGET) $*

%.volt: $(TARGET).c
	$(CC) $(CFLAGS) $(META_OPS) -DCREATE_VOLTAGE_FILE -o $(TARGET) $(TARGET).c
	@echo "built with CREATE_VOLTAGE_FILE"
	./$(TARGET) $*

%.int: $(TARGET).c
	$(CC) $(CFLAGS) $(META_OPS) -DCREATE_INT_FILE -o $(TARGET) $(TARGET).c
	@echo "built with CREATE_INT_FILE"
	./$(TARGET) $*

clean:
	$(RM) *.o *.out *.csv $(TARGET)
	$(RM) -r $(TARGET).dSYM
	@echo "removed executables and csv"

veryclean:
	$(RM) *.o *.out *.BIN *.csv $(TARGET)
	$(RM) -r $(TARGET).dSYM
	@echo "removed executables, csv, and bin"

help:
	@echo ""
	@echo "Hello $(USER), you forgot some targets."
	@echo " If you need instructions to use this Makefile:"
	@echo "        'make <filename>.volt' to build and run voltage"
	@echo "        'make <filename>.int' to build and integer voltage"
	@echo "        'make <filename>.all' to build and run both"
	@echo "    You can also add additional options:"
	@echo "        'make <filename>.<target> options=<option>'"
	@echo "             where <option> is replaced with:"
	@echo "               't' to track extreme deltas"
	@echo "               'z' to remove zero data"
	@echo "               'tz' to do both options"
	@echo " Clean Rules:"
	@echo "        'make clean' to remove executables and csv"
	@echo "        'make veryclean' to remove executables, csv, and bin"
	@echo ""


# depreciated commands
.PHONY : build run

build: $(TARGET).c
	$(CC) $(CFLAGS) $(META_OPS) -o $(TARGET) $(TARGET).c
	@echo "compiled target"

# example execution: make test args=F0
test: build
	./$(TARGET) $(args)
	@echo "ran executable"
