# Compiler setup.
CC=gcc
CFLAGS=-Wall -Wextra -std=c99

# Define targets
TARGETS=runic

# Manually define all targets.
build: $(TARGETS)

runic: runic.c lsys_functions.c undoable_functions.c turtle.c bitcheck.c header.h
	$(CC) $(CFLAGS) runic.c lsys_functions.c undoable_functions.c turtle.c bitcheck.c -o runic -lm
# Pack the solution into a zip file.
pack:
	zip -FSr PPM-Image-Processor-with-LSystems.zip README Makefile *.c *.h

# Clean the solution.
clean:
	rm -f $(TARGETS)

.PHONY: pack clean
