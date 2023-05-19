CC 		:= gcc
CFLAGS 	:= -Werror -Wall -pipe -march=native -O3 #-pg
LDFLAGS := #-lm #-pg
CSUFFIX := .c
HSUFFIX := .h

APPNAME := a

SRC_DIR := src
OBJ_DIR := obj
TRG_DIR := .

.PHONY: clean

TARGET  := $(addprefix $(TRG_DIR)/,$(APPNAME))
C_FILES := $(wildcard $(SRC_DIR)/*$(CSUFFIX))
O_FILES := $(addprefix $(OBJ_DIR)/,$(notdir $(C_FILES:$(CSUFFIX)=.o)))
D_FILES := $(addprefix $(OBJ_DIR)/,$(notdir $(C_FILES:$(CSUFFIX)=.d)))

all: $(TARGET)

# link all .o files
$(TARGET): $(O_FILES) | $(TRG_DIR)
	@echo link "   :" $^
	@$(CC) $(LDFLAGS) -o $@ $^

# depend on include files
-include $(D_FILES)

# compile all .c files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%$(CSUFFIX) Makefile | $(OBJ_DIR)
	@echo compile : $<
	@$(CC) $(CFLAGS) -c -MMD -MP -o $@ $<

# create any missing directories
$(OBJ_DIR):
	@mkdir $@
$(TRG_DIR):
	@mkdir $@

clean:
	@rm -rf $(OBJ_DIR) $(TARGET)
	@if [ ! "$(TRG_DIR)" = "." ]; then \
		rm -rf $(TRG_DIR); \
	fi
