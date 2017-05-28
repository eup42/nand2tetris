CC = gcc
CFLAGS += -Wall -g

TARGET = Assembler
OBJ = assembler.o parser.o code.o


all: $(TARGET)

Assembler: $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

.PHONY: clean
clean:
	rm -f $(TARGET) *.o
