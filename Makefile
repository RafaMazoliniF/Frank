# Compilador
CC = gcc

# --- FLAGS ---
# Flags base, usadas em todas as compilações
BASE_CFLAGS = -Wall -Wextra -std=c99 -Iinclude -Iunity

# Flags de Debug (padrão)
DEBUG_CFLAGS = -g

# Flags de Release (usadas pelo target 'release')
RELEASE_CFLAGS = -O2

# CFLAGS padrão é o modo debug (com -g)
CFLAGS = $(BASE_CFLAGS) $(DEBUG_CFLAGS)

# --- DIRETÓRIOS ---
SRCDIR = src
INCDIR = include
TESTDIR = tests
UNITYDIR = unity

# --- EXECUTÁVEIS ---
TARGET = main
TEST_TARGET = test_runner

# --- ARQUIVOS FONTE E OBJETO (APLICAÇÃO PRINCIPAL) ---
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:.c=.o)
APP_OBJECTS_NO_MAIN = $(filter-out $(SRCDIR)/main.o, $(OBJECTS))

# --- ARQUIVOS FONTE E OBJETO (TESTES E UNITY) ---
UNITY_OBJECT = $(UNITYDIR)/unity.o
TEST_RUNNER_SRC = $(TESTDIR)/test_runner.c
TEST_RUNNER_OBJ = $(TEST_RUNNER_SRC:.c=.o)
TEST_CASE_SOURCES = $(filter-out $(TEST_RUNNER_SRC), $(wildcard $(TESTDIR)/*.c))

# --- REGRAS DE COMPILAÇÃO ---

# Regra padrão: compila o programa principal (em modo debug)
all: $(TARGET)

# Target para compilar em modo DEBUG (com -g)
# Este é o padrão, mas é bom ter um target explícito
debug: $(TARGET)

# Target para compilar em modo RELEASE (com -O2)
# Ele define a variável CFLAGS apenas para esta execução
release: CFLAGS = $(BASE_CFLAGS) $(RELEASE_CFLAGS)
release: all

# Regra para compilar o executável principal
$(TARGET): $(OBJECTS)
	@echo "--- Linkando $(TARGET) com flags: $(CFLAGS) ---"
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

# Regra para compilar o executável de testes
$(TEST_TARGET): $(APP_OBJECTS_NO_MAIN) $(TEST_RUNNER_OBJ) $(UNITY_OBJECT)
	@echo "--- Linkando $(TEST_TARGET) com flags: $(CFLAGS) ---"
	$(CC) $(CFLAGS) $(APP_OBJECTS_NO_MAIN) $(TEST_RUNNER_OBJ) $(UNITY_OBJECT) -o $(TEST_TARGET)

# Regra para compilar arquivos .c da aplicação em .o
$(SRCDIR)/%.o: $(SRCDIR)/%.c $(wildcard $(INCDIR)/*.h)
	@echo "Compilando (app) $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para compilar o test_runner.c em .o
$(TEST_RUNNER_OBJ): $(TEST_RUNNER_SRC) $(TEST_CASE_SOURCES) $(wildcard $(INCDIR)/*.h)
	@echo "Compilando (test runner) $<..."
	$(CC) $(CFLAGS) -c $(TEST_RUNNER_SRC) -o $(TEST_RUNNER_OBJ)

# Regra para compilar o unity.c em .o
$(UNITY_OBJECT): $(UNITYDIR)/unity.c
	@echo "Compilando (unity) $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# --- REGRAS DE EXECUÇÃO E LIMPEZA ---

# Compila (em modo debug, por padrão) e executa os testes
test: $(TEST_TARGET)
	./$(TEST_TARGET)

# Limpar arquivos compilados
clean:
	@echo "Limpando arquivos compilados..."
	rm -f $(SRCDIR)/*.o $(TARGET) res.txt
	rm -f $(TESTDIR)/*.o $(UNITYDIR)/*.o $(TEST_TARGET)

# Executar o programa principal (compila em debug por padrão)
run: $(TARGET)
	./$(TARGET) -v

# Recompilar tudo (em modo debug)
rebuild: clean all

# Declara alvos que não são arquivos
.PHONY: all clean run rebuild test debug release