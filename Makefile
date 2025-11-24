# Makefile Híbrido Windows/Linux

CC = gcc
BASE_CFLAGS = -Wall -Wextra -std=c99 -Iinclude -Iunity

# Diretórios e Arquivos
SRCDIR = src
INCDIR = include
# Pega todos os .c, exceto o main.c original (que é CLI) e o gui_main.c (que tratamos separado)
COMMON_SOURCES = $(filter-out $(SRCDIR)/main.c $(SRCDIR)/gui_main.c, $(wildcard $(SRCDIR)/*.c))
# Adiciona o utilitário de GUI
COMMON_SOURCES += $(SRCDIR)/gui_utils.c
COMMON_OBJECTS = $(COMMON_SOURCES:.c=.o)

TARGET_GUI = frank_ide

# Detecção de SO
ifeq ($(OS),Windows_NT)
    # --- Configuração Windows ---
    EXT = .exe
    GUI_FLAGS = -lgdi32 -mwindows
    RM = del /Q
    FIX_PATH = $(subst /,\,$1)
else
    # --- Configuração Linux ---
    EXT = 
    # Flags do GTK para Linux
    GUI_FLAGS = `pkg-config --cflags --libs gtk+-3.0`
    RM = rm -f
    FIX_PATH = $1
endif

FULL_TARGET = $(TARGET_GUI)$(EXT)

all: gui

# Compilação da Interface Gráfica
gui: $(COMMON_OBJECTS) $(SRCDIR)/gui_main.o
	@echo "--- Compilando GUI para $(if $(filter .exe,$(EXT)),Windows,Linux) ---"
	$(CC) $(BASE_CFLAGS) $(COMMON_OBJECTS) $(SRCDIR)/gui_main.o -o $(FULL_TARGET) $(GUI_FLAGS)

# Compilação de Objetos genéricos
$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(BASE_CFLAGS) $(if $(filter $(SRCDIR)/gui_main.o,$@),$(if $(filter .exe,$(EXT)),,`pkg-config --cflags gtk+-3.0`)) -c $< -o $@

clean:
	$(RM) $(call FIX_PATH,$(SRCDIR)/*.o) $(call FIX_PATH,*.exe) $(call FIX_PATH,*.obj) $(call FIX_PATH,temp_gui.txt) $(call FIX_PATH,$(FULL_TARGET))

.PHONY: all gui clean