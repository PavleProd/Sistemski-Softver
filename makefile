INC = ./inc
SRC = ./src
MISC = ./misc
COMMON = ./common
BUILD	= ./build
BIN = ./bin

# all

CXX = g++
OPT = -O3
DEPFLAGS = -MP -MD
CXXFLAGS = -Wall -Wextra -g $(foreach D,$(INCDIRS), -I$(D)) $(OPT) $(DEPFLAGS)

all: assembler

# asembler

ASM_INC_DIR = $(INC)/assembler
ASM_SRC_DIR = $(SRC)/assembler
ASM_BUILD_DIR = $(BUILD)/$(SRC)/assembler
ASM_BIN_DIR = $(BIN)/assembler

ASM_CXX = $(wildcard $(ASM_SRC_DIR)/*.cpp)
ASM_OBJ = $(patsubst $(ASM_SRC_DIR)/%.cpp, $(ASM_BUILD_DIR)/%.o, $(ASM_CXX))
ASM_DEP = $(patsubst $(ASM_SRC_DIR)/%.cpp, $(ASM_BUILD_DIR)/%.d, $(ASM_CXX))

assembler: $(ASM_BUILD_DIR) $(ASM_BIN_DIR)

$(ASM_BUILD_DIR):
	mkdir -p $@

$(ASM_BIN_DIR): $(ASM_OBJ) $(MISC_OBJ) $(COMMON_OBJ)
	$(CXX) -o $@ $^

$(ASM_BUILD_DIR)/%.o : $(ASM_SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# misc

MISC_INC_DIR = $(MISC)
MISC_SRC_DIR = $(MISC)
MISC_BUILD_DIR = $(BUILD)/misc

MISC_CXX = $(wildcard $(MISC_SRC_DIR)/*.cpp)
MISC_OBJ = $(patsubst $(MISC_SRC_DIR)/%.cpp, $(MISC_BUILD_DIR)/%.o, $(MISC_CXX))
MISC_DEP = $(patsubst $(MISC_SRC_DIR)/%.cpp, $(MISC_BUILD_DIR)/%.d, $(MISC_CXX))

$(MISC_BUILD_DIR)/%.o: $(MISC_SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# common

COMMON_BUILD_DIR = $(BUILD)/$(SRC)/common
COMMON_INC_DIR = $(INC)/common
COMMON_SRC_DIR = $(SRC)/common

COMMON_CXX = $(wildcard $(COMMON_SRC_DIR)/*.cpp)
COMMON_OBJ = $(wildcard $(COMMON_SRC_DIR)/%.cpp, $(COMMON_BUILD_DIR)/%.o, $(COMMON_CXX))
COMMON_DEP = $(wildcard $(COMMON_SRC_DIR)/%.cpp, $(COMMON_BUILD_DIR)/%.o, $(COMMON_CXX))

$(COMMON_BUILD_DIR)/%.o: %(COMMON_SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

-include $(ASM_DEP) $(MISC_DEP) $(COMMON_DEP)

.PHONY: all assembler