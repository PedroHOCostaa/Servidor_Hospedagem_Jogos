# Nome do compilador
CC = gcc

# Flags do compilador
CFLAGS = -Wall -pthread

# Nome do arquivo de saída
TARGET = cliente

# Regra padrão para compilar
all: $(TARGET)

$(TARGET): cliente.o
	$(CC) $(CFLAGS) -o $(TARGET) cliente.o

cliente.o: cliente.c
	$(CC) $(CFLAGS) -c cliente.c

# Regra para limpar os arquivos compilados
clean:
	rm -f *.o $(TARGET)
