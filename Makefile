########################################################################
#                                                                      #
#              Gamma-Ray Spectroscopy at ISAC (GRSI)
#                                                                      #
#   High voltage control 
#                                                                      #
#   Makefile: builds high voltage control program 
#                                                                      #
#   Created: October 2019
#   Last mod: October 2019
#                                                                      #
#   Auth: C. Natzke
#                                                                      #
########################################################################

TARGET_EXEC ?= GRSIHVControl
DIRS = .obj

CC = gcc
SRC_DIR ?= ./src
OBJ_DIR ?= ./.obj

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CPPFLAGS += -Iinclude
CFLAGS   += -DUNIX -DLINUX -Wall -ggdb3
LDFLAGS  +=
LDLIBS   += -lcaenhvwrapper -lncurses -lpthread -ldl -lm

# Makes necesary directory
$(info $(shell mkdir -p $(DIRS)))

################################################################################
.PHONY: all clean

all: $(TARGET_EXEC)

$(TARGET_EXEC): $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ) $(TARGET_EXEC)
