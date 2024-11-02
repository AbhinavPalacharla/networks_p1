CC = gcc
CFLAGS = -Wall -Wextra -g

# Detect operating system
UNAME_S := $(shell uname -s)

# Detect architecture on macOS
ifeq ($(UNAME_S),Darwin)
    # Check if running on Apple Silicon
    UNAME_M := $(shell uname -m)
    ifeq ($(UNAME_M),arm64)
        # M1/M2 Mac
        READLINE_FLAGS = -L/opt/homebrew/opt/readline/lib -I/opt/homebrew/opt/readline/include -lreadline
    else
        # Intel Mac
        READLINE_FLAGS = -L/usr/local/opt/readline/lib -I/usr/local/opt/readline/include -lreadline
    endif
else
    # Linux and other Unix-like systems
    READLINE_FLAGS = -lreadline
endif

# Output executables
SERVER = server
CLIENT = client

# Source files
SERVER_SRC = server.c
CLIENT_SRC = client.c

# Header files
HEADERS = server.h client.h duckchat.h shared.h

# Default target
all: $(SERVER) $(CLIENT)

# Print system information
info:
	@echo "Operating System: $(UNAME_S)"
ifeq ($(UNAME_S),Darwin)
	@echo "Architecture: $(UNAME_M)"
endif
	@echo "Readline Flags: $(READLINE_FLAGS)"

# Server compilation
$(SERVER): $(SERVER_SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(SERVER_SRC) -o $(SERVER)

# Client compilation
$(CLIENT): $(CLIENT_SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(CLIENT_SRC) $(READLINE_FLAGS) -o $(CLIENT)

# Clean build files
clean:
	rm -f $(SERVER) $(CLIENT)

# Rebuild everything
rebuild: clean all

.PHONY: all clean rebuild info
