# Makefile.in generated by automake 1.16.5 from Makefile.am.
# Makefile.  Generated from Makefile.in by configure.

# Copyright (C) 1994-2021 Free Software Foundation, Inc.

# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.

CC = gcc
CFLAGS = -Wall -Wextra -g -DUSE_COMMON
GTK_LIBS = `pkg-config --cflags --libs gtk+-3.0`
SHA256_LIBS = -lssl -lcrypto

SRC_DIR = src
BIN_DIR = bin

# Specify target binaries
TARGETS = $(BIN_DIR)/backup $(BIN_DIR)/restore

# Create bin directory if it doesn't exist
$(shell mkdir -p $(BIN_DIR))

all: $(TARGETS)

# Build backup with optional GUI support
$(BIN_DIR)/backup: $(SRC_DIR)/backup.c $(SRC_DIR)/common.c $(SRC_DIR)/metadata.c $(SRC_DIR)/parity.c $(SRC_DIR)/parity_core.c
	$(CC) $(CFLAGS) -o $@ $^ $(SHA256_LIBS) $(GTK_LIBS)

# Build backup with GUI support
$(BIN_DIR)/backup_gui: $(SRC_DIR)/backup.c $(SRC_DIR)/common.c $(SRC_DIR)/metadata.c $(SRC_DIR)/parity.c $(SRC_DIR)/parity_core.c $(SRC_DIR)/backup_gui.c
	$(CC) $(CFLAGS) -DUSE_GUI -o $@ $^ $(SHA256_LIBS) $(GTK_LIBS)

# Build restore without GUI
$(BIN_DIR)/restore: $(SRC_DIR)/restore.c $(SRC_DIR)/common.c $(SRC_DIR)/metadata.c $(SRC_DIR)/parity.c $(SRC_DIR)/parity_core.c
	$(CC) $(CFLAGS) -o $@ $^ $(SHA256_LIBS)

# To include GUI support in backup, define GUI flag in CFLAGS
gui: $(BIN_DIR)/backup_gui

install:
	@mkdir -p ~/.local/bin
	@{ \
		if [ -f $(BIN_DIR)/backup ]; then \
			cp $(BIN_DIR)/backup ~/.local/bin/; \
		fi; \
		if [ -f $(BIN_DIR)/restore ]; then \
			cp $(BIN_DIR)/restore ~/.local/bin/; \
		fi; \
		if [ -f $(BIN_DIR)/backup_gui ]; then \
			cp $(BIN_DIR)/backup_gui ~/.local/bin/; \
		fi; \
	}
	@{ \
		if ! grep -q 'export PATH="$HOME/.local/bin:$PATH"' ~/.bashrc; then \
			echo 'export PATH="$$HOME/.local/bin:$$PATH"' >> ~/.bashrc; \
		fi; \
	}

clean:
	rm -f $(BIN_DIR)/backup $(BIN_DIR)/restore $(BIN_DIR)/backup_gui
	rm -f $(SRC_DIR)/*.o
