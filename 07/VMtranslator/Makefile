CC = gcc
CFLAGS += -g -Wall

TARGET = VMtranslator
OBJ = parser.o code_writer.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

.PHONY: clean
clean:
	rm -f $(TARGET) *.o
