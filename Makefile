#source: https://spin.atomicobject.com/2016/08/26/makefile-c-projects/

TARGET_EXEC ?= main
TARGET_OBJ ?= obj

BUILD_DIR ?= ./bin
SRC_DIRS ?= ./src

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP -Wall -g -O2 -fdiagnostics-color=always -Iopenssl/openssl-*/include -Lopenssl/openssl-* -pipe -I.
LDFLAGS ?= -lssl -lcrypto -lz -pipe -g -O2 -Wall

CCOMP ?= gcc
CPPCOMP ?= g++

#build executable
$(BUILD_DIR)/$(TARGET_EXEC): $(BUILD_DIR)/$(TARGET_OBJ)
	$(CPPCOMP) $(BUILD_DIR)/$(TARGET_OBJ) -o $@ $(LDFLAGS)

#build linkable binary
$(BUILD_DIR)/$(TARGET_OBJ): $(OBJS)
	ar -rcs $(BUILD_DIR)/$(TARGET_OBJ) $(OBJS)

# assembly
$(BUILD_DIR)/%.s.o: %.s
	$(MKDIR_P) $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CCOMP) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CPPCOMP) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@


.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)
	$(MKDIR_P) $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p
