CC = gcc
CFLAGS += -g -Wall

TARGET = JackCompiler
OBJ = jack_compiler.o jack_tokenizer.o compilation_engine.o symbol_table.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

.PHONY: clean
clean:
	rm -f $(TARGET) *.o
