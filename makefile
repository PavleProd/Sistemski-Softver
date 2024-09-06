INC = inc
SRC = src
MISC = misc
COMMON = common
BUILD = build

# misc

BISON_INPUT = $(MISC)/parser.y
BISON_OUTPUT = $(MISC)/parser.cpp

FLEX_INPUT = $(MISC)/lexer.l
FLEX_OUTPUT = $(MISC)/lexer.cpp

MISC_SRC_FILES = $(BISON_OUTPUT) $(FLEX_OUTPUT)
MISC_OBJ = $(patsubst $(MISC_SRC_DIR)/%.cpp, $(BUILD)/%.o, $(MISC_SRC_FILES))
MISC_DEP = $(patsubst $(MISC_SRC_DIR)/%.cpp, $(BUILD)/%.d, $(MISC_SRC_FILES))

# common promenljive

COMMON_SRC_DIR = $(SRC)/common
COMMON_SRC_FILES = $(wildcard $(COMMON_SRC_DIR)/*.cpp)
COMMON_OBJ = $(patsubst $(COMMON_SRC_DIR)/%.cpp, $(BUILD)/%.o, $(COMMON_SRC_FILES))
COMMON_DEP = $(patsubst $(COMMON_SRC_DIR)/%.cpp, $(BUILD)/%.o, $(COMMON_SRC_FILES))

# asm promenljive

ASM_SRC_DIR = $(SRC)/assembler
ASM_SRC_FILES = $(wildcard $(ASM_SRC_DIR)/*.cpp)
ASM_OBJ = $(patsubst $(ASM_SRC_DIR)/%.cpp, $(BUILD)/%.o, $(ASM_SRC_FILES))
ASM_OBJ += $(MISC_OBJ)
ASM_OBJ += $(COMMON_OBJ)

ASM_DEP = $(patsubst $(ASM_SRC_DIR)/%.cpp, $(BUILD)/%.d, $(ASM_SRC_FILES))

# recepti

CXX = g++ -std=c++17
#OPT = -O3
DEPFLAGS = -MMD -MP
CXXFLAGS = -Wall -Wextra -g -I$(INC)

all: assembler

# debug: $(info ASM_OBJ: $(ASM_OBJ))

assembler: $(ASM_OBJ)
	$(CXX) -o $@ $^

# build recepti

$(BUILD):
	mkdir -p $@

$(BUILD)/%.o: $(COMMON_SRC_DIR)/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -o $@ -c $<

$(BUILD)/%.o: $(ASM_SRC_DIR)/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -o $@ -c $<

$(BUILD)/%.o: $(MISC_SRC_DIR)/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -o $@ -c $<

-include $(ASM_DEP) $(MISC_DEP) $(COMMON_DEP)

clean:
	$(info "USAO")
	rm -rf assembler
	rm -rf $(BUILD)

.PHONY: all assembler clean
