CC=gcc
CFLAGS=
TARGET= myshell
OBJ=myshell.o

all: $(TARGET)


$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

$(OBJ): myshell.c
	$(CC) $(CFLAGS) -c myshell.c
	
clean:
	rm -f $(TARGET) *.o
