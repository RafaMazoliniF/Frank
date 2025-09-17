# Compilador e flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude

# Diretórios
SRCDIR = src
INCDIR = include

# Nome do executável
TARGET = lexical_analyzer

# Arquivos fonte
SOURCES = $(wildcard $(SRCDIR)/*.c)

# Arquivos objeto (colocados na pasta src)
OBJECTS = $(SOURCES:.c=.o)

# Regra principal
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

# Regra para compilar .c em .o
$(SRCDIR)/%.o: $(SRCDIR)/%.c $(INCDIR)/*.h
	$(CC) $(CFLAGS) -c $< -o $@

# Limpar arquivos compilados
clean:
	rm -f $(SRCDIR)/*.o $(TARGET) res.txt

# Executar
run: $(TARGET)
	./$(TARGET)

# Recompilar tudo
rebuild: clean $(TARGET)

.PHONY: clean run rebuild