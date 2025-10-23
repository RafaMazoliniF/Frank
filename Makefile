# Compilador e flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude -Iunity

# --- DIRETÓRIOS ---
SRCDIR = src
INCDIR = include
TESTDIR = tests
UNITYDIR = unity

# --- EXECUTÁVEIS ---
TARGET = lexical_analyzer
TEST_TARGET = test_runner

# --- ARQUIVOS FONTE E OBJETO (APLICAÇÃO PRINCIPAL) ---
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:.c=.o)

# CORREÇÃO: Cria uma lista de objetos da aplicação, mas EXCLUINDO o main.o
# Isso é usado para construir o runner de testes, que tem seu próprio main.
APP_OBJECTS_NO_MAIN = $(filter-out $(SRCDIR)/main.o, $(OBJECTS))

# --- ARQUIVOS FONTE E OBJETO (TESTES E UNITY) ---
TEST_SOURCES = $(wildcard $(TESTDIR)/*.c)
UNITY_SOURCES = $(UNITYDIR)/unity.c
TEST_OBJECTS = $(TEST_SOURCES:.c=.o) $(UNITY_SOURCES:.c=.o)

# --- REGRAS DE COMPILAÇÃO ---

# Regra padrão: compila o programa principal
all: $(TARGET)

# Regra para compilar o executável principal
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

# Regra para compilar o executável de testes
# CORREÇÃO: Usa a lista de objetos SEM o main.o para a linkagem
$(TEST_TARGET): $(APP_OBJECTS_NO_MAIN) $(TEST_OBJECTS)
	$(CC) $(APP_OBJECTS_NO_MAIN) $(TEST_OBJECTS) -o $(TEST_TARGET)

# Regra para compilar arquivos .c da aplicação em .o
$(SRCDIR)/%.o: $(SRCDIR)/%.c $(wildcard $(INCDIR)/*.h)
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para compilar arquivos .c de teste em .o
$(TESTDIR)/%.o: $(TESTDIR)/%.c $(wildcard $(INCDIR)/*.h)
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para compilar o unity.c em .o
$(UNITYDIR)/%.o: $(UNITYDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# --- REGRAS DE EXECUÇÃO E LIMPEZA ---

# Compila e executa os testes
test: $(TEST_TARGET)
	./$(TEST_TARGET) 

# Limpar arquivos compilados
clean:
	rm -f $(SRCDIR)/*.o $(TARGET) res.txt
	rm -f $(TESTDIR)/*.o $(UNITYDIR)/*.o $(TEST_TARGET)

# Executar o programa principal
run: $(TARGET)
	./$(TARGET) -v

# Recompilar tudo (programa principal)
rebuild: clean all

# Declara alvos que não são arquivos
.PHONY: all clean run rebuild test
