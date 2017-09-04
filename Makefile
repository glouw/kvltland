# Compiler and standard
CC = gcc -std=c99

# Project name
PROJ = land

# Source files
SRCS = main.c

# Warnings flags
CFLAGS = -Wshadow -Wall -Wpedantic -Wextra -Wpadded

# Debugging flags
CFLAGS+= -g

# Optimization flags
CFLAGS+= -Ofast -flto

# Architecture flags
ARCH = -march=native -m32

# Linker flags
LDFLAGS = -lSDL2

# Linker
$(PROJ): $(SRCS:.c=.o)
	$(CC) $(ARCH) $(CFLAGS) $(SRCS:.c=.o) $(LDFLAGS) -o $(PROJ)

# Compiler template; generates dependency targets
%.o : %.c
	$(CC) $(ARCH) $(CFLAGS) -MT $@ -MMD -MP -MF $*.td -c $<
	@mv -f $*.td $*.d

# All dependency targets
%.d: ;
-include *.d

clean:
	rm -f $(PROJ)
	rm -f $(SRCS:.c=.o)
	rm -f $(SRCS:.c=.d)
