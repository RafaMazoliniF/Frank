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
APP_OBJECTS_NO_MAIN = $(filter-out $(SRCDIR)/main.o, $(OBJECTS))

# --- ARQUIVOS FONTE E OBJETO (TESTES E UNITY) ---

# O objeto do Unity
UNITY_OBJECT = $(UNITYDIR)/unity.o

# O arquivo .c principal do runner de testes
TEST_RUNNER_SRC = $(TESTDIR)/test_runner.c
# O objeto principal do runner de testes
TEST_RUNNER_OBJ = $(TEST_RUNNER_SRC:.c=.o)

# Lista de todos os arquivos .c de casos de teste (que são #included pelo runner)
TEST_CASE_SOURCES = $(filter-out $(TEST_RUNNER_SRC), $(wildcard $(TESTDIR)/*.c))

# --- REGRAS DE COMPILAÇÃO ---

# Regra padrão: compila o programa principal
all: $(TARGET)

# Regra para compilar o executável principal
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

# Regra para compilar o executável de testes
# CORREÇÃO: Linka apenas os objetos da app, o runner principal e o unity.
$(TEST_TARGET): $(APP_OBJECTS_NO_MAIN) $(TEST_RUNNER_OBJ) $(UNITY_OBJECT)
	$(CC) $(APP_OBJECTS_NO_MAIN) $(TEST_RUNNER_OBJ) $(UNITY_OBJECT) -o $(TEST_TARGET)

# Regra para compilar arquivos .c da aplicação em .o
$(SRCDIR)/%.o: $(SRCDIR)/%.c $(wildcard $(INCDIR)/*.h)
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para compilar o test_runner.c em .o
# CORREÇÃO: Esta é a única regra que compila um .c de teste.
# Ela depende dos arquivos de caso de teste (.c) que ela inclui.
$(TEST_RUNNER_OBJ): $(TEST_RUNNER_SRC) $(TEST_CASE_SOURCES) $(wildcard $(INCDIR)/*.h)
	$(CC) $(CFLAGS) -c $(TEST_RUNNER_SRC) -o $(TEST_RUNNER_OBJ)

# Regra para compilar o unity.c em .o
$(UNITY_OBJECT): $(UNITYDIR)/unity.c
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