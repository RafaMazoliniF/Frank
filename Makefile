# Makefile Híbrido Windows/Linux

CC = gcc
BASE_CFLAGS = -Wall -Wextra -std=c99 -Iinclude -Iunity

# Diretórios
SRCDIR = src
INCDIR = include
# Filtra main.c (CLI) e gui_main.c (GUI), pois são compilados separadamente ou explicitamente
COMMON_SOURCES = $(filter-out $(SRCDIR)/main.c $(SRCDIR)/gui_main.c, $(wildcard $(SRCDIR)/*.c))
COMMON_OBJECTS = $(COMMON_SOURCES:.c=.o)

TARGET_GUI = frank_ide

# Detecção de SO e Configurações Específicas
ifeq ($(OS),Windows_NT)
    # --- Windows ---
    EXT = .exe
    # -lcomdlg32: Necessário para abrir/salvar arquivos
    # -fexec-charset=cp1252: Corrige acentuação no Windows
    GUI_FLAGS = -lgdi32 -mwindows -lcomdlg32
    BASE_CFLAGS += -finput-charset=UTF-8 -fexec-charset=cp1252
    RM = del /Q
    FIX_PATH = $(subst /,\,$1)
else
    # --- Linux ---
    EXT = 
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