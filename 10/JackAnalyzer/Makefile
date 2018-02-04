CC = gcc
CFLAGS += -g -Wall

TARGET = JackAnalyzer
OBJ = jack_analyzer.o jack_tokenizer.o compilation_engine.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

.PHONY: clean
clean:
	rm -f $(TARGET) *.o
