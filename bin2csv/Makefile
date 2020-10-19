CC = gcc
CFLAGS = -g -Wall
TARGET = conversion

.PHONY : all run

all: $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c
	@echo "compiled target"

clean:
	$(RM) *.o *.out *.csv
	@echo "removed executables and csv"

veryclean:
	$(RM) *.o *.out *.BIN *.csv
	@echo "removed executables, csv, and bin"

# example execution: make test args=F0
test: all
	./$(TARGET) $(args)
	@echo "ran executable"
