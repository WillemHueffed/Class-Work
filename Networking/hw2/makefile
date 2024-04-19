# Compiler
CC = gcc

# Compiler flags
#CFLAGS = -Wall -Wextra -g
CFLAGS = -g

LIBS = -pthread

# Source files for the server and client
SERVER_SRCS = server.c
CLIENT_SRCS = client.c
OTHER_SRCS = fib.c

# Object files for the server and client
SERVER_OBJS = $(SERVER_SRCS:.c=.o)
CLIENT_OBJS = $(CLIENT_SRCS:.c=.o)
OTHER_OBJS= $(OTHER_SRCS:.c=.o)

# Executables for the server and client
SERVER_EXEC = server 
CLIENT_EXEC = client 
OTHER_EXEC = fib.cgi

# Default target
all: $(SERVER_EXEC) $(CLIENT_EXEC) $(OTHER_EXEC)

# Server executable
$(SERVER_EXEC): $(SERVER_OBJS)
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@

# Client executable
$(CLIENT_EXEC): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(OTHER_EXEC): $(OTHER_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Compilation rules
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(SERVER_EXEC) $(CLIENT_EXEC) $(OTHER_EXEC) \
		$(SERVER_OBJS) $(CLIENT_OBJS) $(OTHER_OBJS) a.out

