# Makefile for VBCC AmigaOS 3.1 with MUI 3.8 project (NDK 3.2)

# Compiler and tools
CC = vc
RM = rm -f

# Directories
SRCDIR = src
INCDIR = include
BINDIR = bin
OBJDIR = obj
UTILSDIR = src/utils
VIEWSDIR = src/views
WIDGETSDIR = src/widgets

# Target executable
TARGET = $(BINDIR)/main

# Compiler flags for NDK 3.2
CFLAGS = +aos68k \
         -I/opt/sdk/NDK3.2/Include_H \
         -I/opt/vbcc/targets/m68k-amigaos/include \
         -I$(INCDIR) \
         -I/opt/sdk/MUI_3.8/C/Include \
         -O2 -c99

# Library flags for NDK 3.2
LDFLAGS = -L/opt/vbcc/targets/m68k-amigaos/lib \
          -L/opt/sdk/NDK3.2/lib \
          -lamiga -lauto

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.c) $(wildcard $(UTILSDIR)/*.c) $(wildcard $(VIEWSDIR)/*.c) $(wildcard $(WIDGETSDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Default target
all: directories $(TARGET)

# Show available libraries (for debugging)
show-libs:
	@echo "Available libraries in /opt/vbcc/targets/m68k-amigaos/lib:"
	@ls -la /opt/vbcc/targets/m68k-amigaos/lib/ | grep -E "(amiga|auto)" || echo "No amiga libraries found"
	@echo "NDK 3.2 lib contents:"
	@ls -la /opt/sdk/NDK3.2/lib/ 2>/dev/null || echo "NDK lib dir not accessible"
	@echo "MUI include dir contents:"
	@ls -la /opt/sdk/MUI_3.8/C/Include/ 2>/dev/null || echo "MUI include dir not accessible"

# Create directories if they don't exist
directories:
	@mkdir -p $(BINDIR) $(OBJDIR)

# Link the executable
$(TARGET): $(OBJECTS)
	$(CC) +aos68k $(OBJECTS) -o $@ $(LDFLAGS)

# Compile source files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $< -c -o $@

# Clean build artifacts
clean:
	$(RM) $(OBJECTS) $(TARGET)

# Force rebuild
rebuild: clean all

# Quick compile single file (for testing)
quick: directories
	$(CC) +aos68k -I/opt/sdk/NDK3.2/Include_H -I/opt/vbcc/targets/m68k-amigaos/include -I$(INCDIR) -O2 -c99 $(SRCDIR)/main.c -o $(BINDIR)/main $(LDFLAGS)

# Test NDK headers specifically
test-headers:
	@echo "Testing NDK 3.2 header access:"
	@ls -la /opt/sdk/NDK3.2/Include_H/exec/types.h || echo "exec/types.h not found"
	@ls -la /opt/sdk/NDK3.2/Include_H/exec/memory.h || echo "exec/memory.h not found"

# Debug target - show all environment and paths
debug:
	@echo "VBCC: $$VBCC"
	@echo "PATH: $$PATH"
	@echo "Compiler version:"
	@$(CC) -help 2>/dev/null || echo "Cannot get compiler help"
	@echo "Include paths being used:"
	@echo "  /opt/sdk/NDK3.2/Include_H"
	@echo "  /opt/vbcc/targets/m68k-amigaos/include"
	@echo "  $(INCDIR)"
	@echo "  /opt/sdk/MUI_3.8/C/Include"

.PHONY: all clean rebuild directories quick show-libs debug test-headers