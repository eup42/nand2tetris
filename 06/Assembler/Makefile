
TARGET = Assembler
OBJ = assembler.o parser.o


all: $(TARGET)

Assembler: $(OBJ)
	$(CC) -o $(TARGET) $(OBJ)

.PHONY: clean
clean:
	rm -f $(TARGET) *.o
