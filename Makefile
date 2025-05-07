# Nome do compilador
CC = gcc

# Flags de compilação
CFLAGS = -Wall -Wextra -pthread

# Nome do executável
TARGET = ex1

# Arquivos-fonte
SRCS = main.c passa_tempo.c

# Arquivos objeto
OBJS = $(SRCS:.c=.o)

# Regra padrão (default)
all: build

# Regra de build
build: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para limpar os arquivos temporários
clean:
	rm -f $(OBJS) $(TARGET) *~ *.tmp

.PHONY: all build clean