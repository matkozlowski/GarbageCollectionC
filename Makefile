# Compiler and linker
CC=gcc

# Compiler flags, include the directory for header files
CFLAGS=-Iinclude

# Target executable name
TARGET=main

# Build directories for binaries and object files
BINDIR=build/bin
OBJDIR=build/obj

# Source and object files
SRCS=$(wildcard src/*.c)
OBJS=$(patsubst src/%.c,$(OBJDIR)/%.o,$(SRCS))

# Default make
all: $(BINDIR)/$(TARGET)

# Linking the object files to make the executable
$(BINDIR)/$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Compiling source files to object files
$(OBJDIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Cleaning up the build files
clean:
	rm -rf $(OBJDIR)/*.o $(BINDIR)/$(TARGET)

# Making the directories for binaries and objects if they don't exist
$(shell mkdir -p $(BINDIR) $(OBJDIR))
