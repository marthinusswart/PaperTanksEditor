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
	@$(MKDIR) $(BINDIR) $(OBJDIR)

# Link the executable
$(TARGET): $(OBJECTS)
	$(CC) +aos68k $(OBJECTS) -o $@ $(LDFLAGS)

# Compile paletteanalyzer.c
$(OBJDIR)/paletteanalyzer.o: $(SRCDIR)/paletteanalyzer.c
	$(CC) $(CFLAGS) $< -c -o $@

# Compile ilbmanalyzer.c
$(OBJDIR)/ilbmanalyzer.o: $(SRCDIR)/ilbmanalyzer.c
	$(CC) $(CFLAGS) $< -c -o $@

# Compile filelogger.c
$(OBJDIR)/filelogger.o: $(UTILSDIR)/filelogger.c
	$(CC) $(CFLAGS) $< -c -o $@

# Clean build artifacts
clean:
	$(RM) $(OBJECTS) $(TARGET)

# Force rebuild
rebuild: clean all

# Run the analyzer on the default image
run: all
	$(TARGET) "$(MAINDIR)/assets/disk-space.ilbm"

# Show command help
help:
	@echo "Palette Analyzer Utility Makefile"
	@echo "--------------------------------"
	@echo "make         - Build the utility"
	@echo "make clean   - Remove build files"
	@echo "make rebuild - Force a complete rebuild"
	@echo "make run     - Build and run on the default image"
	@echo "make help    - Show this help message"

.PHONY: all clean rebuild directories run help
