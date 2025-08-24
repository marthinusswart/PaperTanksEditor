# Makefile for VBCC AmigaOS LDFLAGS = -L/opt/vbcc/targets/m68k-amigaos/lib \
          -L/opt/sdk/MUI_3.8/lib \
          -lamiga -lauto with MUI 3.8 project (NDK 3.2)

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
GRAPHICSDIR = src/graphics
EXTERNAL_FROZEN = external/frozen
EXTERNAL_FROZEN_INC = external/frozen
EXTERNAL_LODEPNG = external/lodepng
EXTERNAL_PUFF = external/zlib/

# Target executable
TARGET = $(BINDIR)/main

# Compiler flags for NDK 3.2
CFLAGS = +aos68k \
         -I/opt/sdk/NDK3.2/Include_H \
         -I/opt/vbcc/targets/m68k-amigaos/include \
         -I$(INCDIR) \
         -I/opt/sdk/MUI_3.8/C/Include \
		 -I$(EXTERNAL_FROZEN_INC) \
		 -O2 -c99

# Library flags for NDK 3.2
LDFLAGS = -L/opt/vbcc/targets/m68k-amigaos/lib \
          -L/opt/sdk/NDK3.2/lib \
          -lamiga -lauto

MAIN_SOURCES = $(SRCDIR)/main.c
UTILS_SOURCES = $(UTILSDIR)/*.c $(UTILSDIR)/zlib/*.c
VIEWS_SOURCES = $(VIEWSDIR)/*.c
WIDGETS_SOURCES = $(WIDGETSDIR)/*.c
GRAPHICS_SOURCES = $(wildcard $(GRAPHICSDIR)/*.c) $(wildcard $(GRAPHICSDIR)/png/*.c)

FROZEN_SOURCES = $(EXTERNAL_FROZEN)/*.c
PUFF_SOURCES = $(EXTERNAL_PUFF)/*.c
LODEPNG_SOURCES = $(EXTERNAL_LODEPNG)/*.c
FROZEN_SOURCES = $(wildcard $(EXTERNAL_FROZEN)/*.c)
FROZEN_OBJECTS = $(patsubst external/frozen/%.c, obj/external/frozen/%.o, $(FROZEN_SOURCES))

# Object files
MAIN_OBJECTS = $(OBJDIR)/main.o
UTILS_SOURCES_LIST = $(filter-out $(UTILSDIR)/zlib/%.c, $(wildcard $(UTILSDIR)/*.c))
UTILS_OBJECTS = $(patsubst src/utils/%.c, obj/utils/%.o, $(UTILS_SOURCES_LIST))
ZLIB_SOURCES_LIST = $(wildcard $(UTILSDIR)/zlib/*.c)
ZLIB_OBJECTS = $(patsubst src/utils/zlib/%.c, obj/utils/zlib/%.o, $(ZLIB_SOURCES_LIST))
VIEWS_OBJECTS = $(patsubst src/views/%.c, obj/views/%.o, $(wildcard $(VIEWSDIR)/*.c))
WIDGETS_OBJECTS = $(patsubst src/widgets/%.c, obj/widgets/%.o, $(wildcard $(WIDGETSDIR)/*.c))
GRAPHICS_OBJECTS = $(patsubst src/%, obj/%, $(GRAPHICS_SOURCES:.c=.o))

FROZEN_OBJECTS = $(patsubst external/frozen/%.c, obj/external/frozen/%.o, $(FROZEN_SOURCES))
PUFF_OBJECTS = $(patsubst external/zlib/%.c, obj/external/zlib/%.o, $(PUFF_SOURCES))
LODEPNG_OBJECTS = $(patsubst external/lodepng/%.c, obj/external/lodepng/%.o, $(LODEPNG_SOURCES))
PUFF_SOURCES = $(filter-out $(EXTERNAL_PUFF)/pufftest.c, $(wildcard $(EXTERNAL_PUFF)/*.c))
PUFF_OBJECTS = $(patsubst external/zlib/%.c, obj/external/zlib/%.o, $(PUFF_SOURCES))
LODEPNG_SOURCES = $(wildcard $(EXTERNAL_LODEPNG)/*.c)
LODEPNG_OBJECTS = $(patsubst external/lodepng/%.c, obj/external/lodepng/%.o, $(LODEPNG_SOURCES))

# All objects
OBJECTS = $(MAIN_OBJECTS) $(UTILS_OBJECTS) $(ZLIB_OBJECTS) $(VIEWS_OBJECTS) $(WIDGETS_OBJECTS) $(GRAPHICS_OBJECTS) $(FROZEN_OBJECTS) $(PUFF_OBJECTS) $(LODEPNG_OBJECTS)

# Default target
all: directories $(TARGET) copy-assets
	@echo "========================= NOTICE ============================="
	@echo "Running requires an Amiga emulator (UAE, FS-UAE, WinUAE, etc.)"
	@echo "The compiled binary is at: $(TARGET)"
	@echo "=========================================================="

# Copy assets to bin directory
copy-assets:
	@echo "Copying PNG assets with directory structure preserved..."
	@if [ -d "app/assets" ]; then \
		mkdir -p $(BINDIR)/assets; \
		(cd app && find assets -name "*.png" -type f -exec cp --parents {} ../$(BINDIR)/ \; ); \
		# Move assets directly under bin, remove any app/ prefix if present \
		if [ -d "$(BINDIR)/app/assets" ]; then \
			mv $(BINDIR)/app/assets/* $(BINDIR)/assets/; \
			rm -rf $(BINDIR)/app; \
		fi; \
		echo "  PNG assets copied"; \
	else \
		echo "  Assets directory not found"; \
	fi

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
	@echo "Creating required directories..."
	@mkdir -p $(BINDIR) $(BINDIR)/assets
	@mkdir -p $(OBJDIR) $(OBJDIR)/utils $(OBJDIR)/views $(OBJDIR)/widgets $(OBJDIR)/graphics

# Link the executable
$(TARGET): directories $(OBJECTS)
	$(CC) +aos68k $(OBJECTS) -o $@ $(LDFLAGS)

# Compile source files - Main files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $< -c -o $@


# Compile source files - Utils files
$(OBJDIR)/utils/%.o: $(UTILSDIR)/%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $< -c -o $@

# Compile source files - Utils/zlib files
$(OBJDIR)/utils/zlib/%.o: $(UTILSDIR)/zlib/%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $< -c -o $@

# Compile source files - Views files
$(OBJDIR)/views/%.o: $(VIEWSDIR)/%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $< -c -o $@


# Compile source files - Graphics files
$(OBJDIR)/graphics/%.o: $(GRAPHICSDIR)/%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $< -c -o $@

# Compile source files - Graphics/png files
$(OBJDIR)/graphics/png/%.o: $(GRAPHICSDIR)/png/%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $< -c -o $@

# Compile source files - Widgets files
$(OBJDIR)/widgets/%.o: $(WIDGETSDIR)/%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $< -c -o $@

# Compile external/frozen files
$(OBJDIR)/external/frozen/%.o: $(EXTERNAL_FROZEN)/%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $< -c -o $@

# Compile external/zlib files
$(OBJDIR)/external/zlib/%.o: $(EXTERNAL_PUFF)/%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $< -c -o $@

# Compile external/lodepng files
$(OBJDIR)/external/lodepng/%.o: $(EXTERNAL_LODEPNG)/%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $< -c -o $@

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(OBJDIR)/* 2>/dev/null || true
	@rm -f $(BINDIR)/main 2>/dev/null || true
	@echo "Removed object files and main application binary"

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

# Build without MUI support
basic: CFLAGS += -DNO_MUI_SUPPORT
basic: directories
	@echo "Building without MUI support..."
	@$(MAKE) clean
	@$(MAKE) all CFLAGS="$(CFLAGS) -DNO_MUI_SUPPORT"
	@mv $(BINDIR)/main $(BINDIR)/main-basic
	@echo "========================= NOTICE ============================="
	@echo "Running requires an Amiga emulator (UAE, FS-UAE, WinUAE, etc.)"
	@echo "The compiled binary is at: $(BINDIR)/main-basic"
	@echo "=========================================================="

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
	@echo "2. Configure with Workbench 3.1 and at least 2MB Chip RAM + 4MB Fast RAM"
	@echo "3. Copy the entire 'bin' directory to the emulated Amiga environment"
	@echo "4. Run the application from AmigaDOS CLI/Shell"
	@echo ""
	@echo "The compiled binaries are at:"
	@echo "  - Main application: $(TARGET)"
	@echo "  - No-MUI version: $(BINDIR)/main-basic (if built with 'make basic')"
	@echo "=================================================================="

.PHONY: all clean rebuild directories quick show-libs debug test-headers basic copy-assets emulator-info