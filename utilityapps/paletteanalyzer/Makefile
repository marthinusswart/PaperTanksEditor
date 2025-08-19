# Makefile for ILBM Palette Analyzer utility
# This Makefile is specific to the paletteanalyzer utility app

# Compiler and tools
CC = vc
RM = rm -f
MKDIR = mkdir -p

# Directories
MAINDIR = ../..
SRCDIR = .
BINDIR = bin
OBJDIR = obj
UTILSDIR = $(MAINDIR)/src/utils

# Target executable
TARGET = $(BINDIR)/paletteanalyzer

# Compiler flags for NDK 3.2
CFLAGS = +aos68k \
         -I/opt/sdk/NDK3.2/Include_H \
         -I/opt/vbcc/targets/m68k-amigaos/include \
         -I$(MAINDIR)/include \
         -I$(MAINDIR) \
         -O2 -c99

# Library flags for NDK 3.2
LDFLAGS = -L/opt/vbcc/targets/m68k-amigaos/lib \
          -L/opt/sdk/NDK3.2/lib \
          -lamiga -lauto

# Source files
SOURCES = $(SRCDIR)/paletteanalyzer.c \
          $(SRCDIR)/ilbmanalyzer.c \
          $(UTILSDIR)/filelogger.c

# Object files
OBJECTS = $(OBJDIR)/paletteanalyzer.o \
          $(OBJDIR)/ilbmanalyzer.o \
          $(OBJDIR)/filelogger.o

# Default target
all: directories $(TARGET)

# Create directories if they don't exist
directories:
	@echo "Creating required directories..."
	@$(MKDIR) $(BINDIR) $(OBJDIR)

# Link the executable
$(TARGET): directories $(OBJECTS)
	$(CC) +aos68k $(OBJECTS) -o $@ $(LDFLAGS)

# Compile paletteanalyzer.c
$(OBJDIR)/paletteanalyzer.o: $(SRCDIR)/paletteanalyzer.c
	@$(MKDIR) $(@D)
	$(CC) $(CFLAGS) $< -c -o $@

# Compile ilbmanalyzer.c
$(OBJDIR)/ilbmanalyzer.o: $(SRCDIR)/ilbmanalyzer.c
	@$(MKDIR) $(@D)
	$(CC) $(CFLAGS) $< -c -o $@

# Compile filelogger.c
$(OBJDIR)/filelogger.o: $(UTILSDIR)/filelogger.c
	@$(MKDIR) $(@D)
	$(CC) $(CFLAGS) $< -c -o $@

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(OBJDIR)/* $(BINDIR)/* 2>/dev/null || true

# Force rebuild
rebuild: clean all

# Run the analyzer on the default image
run:
	@echo "========================= NOTICE ============================="
	@echo "Running requires an Amiga emulator (UAE, FS-UAE, WinUAE, etc.)"
	@echo "The compiled binary is at: $(TARGET)"
	@echo "To use this application:"
	@echo "1. Copy the binary and assets to your Amiga/emulator environment"
	@echo "2. Run from AmigaDOS with: paletteanalyzer <ilbm_file_path>"
	@echo "=========================================================="

# Show command help
help:
	@echo "Palette Analyzer Utility Makefile"
	@echo "--------------------------------"
	@echo "make         - Build the utility"
	@echo "make clean   - Remove build files"
	@echo "make rebuild - Force a complete rebuild"
	@echo "make run     - Show information about running in an emulator"
	@echo "make help    - Show this help message"
	@echo "make emulator-info - Show detailed emulator information"

# Information about running in an emulator
emulator-info:
	@echo "========================= EMULATOR INFO ============================="
	@echo "To run this Amiga application, you need an emulator like:"
	@echo "  - FS-UAE: https://fs-uae.net/"
	@echo "  - WinUAE: https://www.winuae.net/ (Windows)"
	@echo "  - UAE: http://uae-emulator.sourceforge.net/"
	@echo ""
	@echo "Basic setup instructions:"
	@echo "1. Install an Amiga emulator"
	@echo "2. Configure with Workbench 3.1 and at least 2MB Chip RAM + 2MB Fast RAM"
	@echo "3. Copy the binary from 'bin/paletteanalyzer' to the emulated Amiga environment"
	@echo "4. Copy the test image from '../../assets/disk-space.ilbm' to the same location"
	@echo "5. Run from AmigaDOS CLI/Shell with: paletteanalyzer disk-space.ilbm"
	@echo "=================================================================="

.PHONY: all clean rebuild directories run help emulator-info
