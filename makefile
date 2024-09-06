INC = inc
SRC = src
MISC = misc
COMMON = common
BUILD = build

# misc

MISC_SRC_DIR = $(MISC)
MISC_SRC_FILES = $(wildcard $(MISC_SRC_DIR)/*.cpp)
MISC_OBJ = $(patsubst $(MISC_SRC_DIR)/%.cpp, $(BUILD)/%.o, $(MISC_SRC_FILES))
MISC_DEP = $(patsubst $(MISC_SRC_DIR)/%.cpp, $(BUILD)/%.d, $(MISC_SRC_FILES))

BISON_INPUT = $(MISC)/parser.y
BISON_OUTPUT = $(MISC)/parser.cpp

FLEX_INPUT = $(MISC)/lexer.l
FLEX_OUTPUT = $(MISC)/lexer.cpp

$(BISON_OUTPUT): $(BISON_INPUT)
	bison -d $^

$(FLEX_OUTPUT): $(FLEX_INPUT)
	flex $^

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

clean:
	rm -rf assembler
	rm -rf $(BUILD)
	rm -rf $(MISC)/*.hpp $(MISC)/*.cpp

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
