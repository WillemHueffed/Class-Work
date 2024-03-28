# Compiler
CC = gcc
# Compiler flags
#CFLAGS = -Wall -Wextra -g
CFLAGS = -g

# Source files for the server and client
SERVER_SRCS = whoisserver.c
CLIENT_SRCS = whoisclient.c

# Object files for the server and client
SERVER_OBJS = $(SERVER_SRCS:.c=.o)
CLIENT_OBJS = $(CLIENT_SRCS:.c=.o)

# Executables for the server and client
SERVER_EXEC = server 
CLIENT_EXEC = client 

# Default target
all: $(SERVER_EXEC) $(CLIENT_EXEC)

# Server executable
$(SERVER_EXEC): $(SERVER_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Client executable
$(CLIENT_EXEC): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Compilation rules
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(SERVER_EXEC) $(CLIENT_EXEC) $(SERVER_OBJS) $(CLIENT_OBJS)

