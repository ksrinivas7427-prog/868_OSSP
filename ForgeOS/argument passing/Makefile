# ==============================================================================
# Makefile for Argument Passing Mini-Shell Project
# Operating Systems & Systems Programming
# ==============================================================================

CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99 -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE

TARGETS = mini_shell argtest

all: $(TARGETS)

mini_shell: mini_shell.c
	$(CC) $(CFLAGS) -o mini_shell mini_shell.c

argtest: argtest.c
	$(CC) $(CFLAGS) -o argtest argtest.c

run: all
	./mini_shell

clean:
	rm -f $(TARGETS) *.o

help:
	@echo "Available targets:"
	@echo "  make        : Build both mini_shell and argtest"
	@echo "  make run    : Build and start mini_shell"
	@echo "  make clean  : Remove compiled binaries and object files"
	@echo "  make help   : Display this help message"

.PHONY: all run clean help
