INC_DIR = inc
SRC_DIR = src
MISC_DIR = misc
OBJ_DIR = obj

BISON_INPUT = $(MISC_DIR)/parser.y
BISON_OUTPUT = $(MISC_DIR)/parser.cpp

FLEX_INPUT = $(MISC_DIR)/lexer.l
FLEX_OUTPUT = $(MISC_DIR)/lexer.cpp

MISC_SRCS = $(BISON_OUTPUT) $(FLEX_OUTPUT)
MISC_OBJ = $(patsubst $(MISC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(MISC_SRCS))
MISC_DEP = $(patsubst $(MISC_DIR)/%.cpp, $(OBJ_DIR)/%.d, $(MISC_SRCS))

COMMON_DIR = $(SRC_DIR)/common
COMMON_SRCS = $(wildcard $(COMMON_DIR)/*.cpp)
COMMON_OBJ = $(patsubst $(COMMON_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(COMMON_SRCS))
COMMON_DEP = $(patsubst $(COMMON_DIR)/%.cpp, $(OBJ_DIR)/%.d, $(COMMON_SRCS))

ASM_DIR = $(SRC_DIR)/assembler
ASM_SRCS = $(wildcard $(ASM_DIR)/*.cpp)
ASM_OBJ = $(patsubst $(ASM_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(ASM_SRCS))
ASM_OBJ += $(MISC_OBJ)
ASM_OBJ += $(COMMON_OBJ)

ASM_DEP = $(patsubst $(ASM_DIR)/%.cpp, $(OBJ_DIR)/%.d, $(ASM_SRCS))

LINKER_DIR = $(SRC_DIR)/linker
LINKER_SRCS = $(wildcard $(LINKER_DIR)/*.cpp)
LINKER_OBJ = $(patsubst $(LINKER_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(LINKER_SRCS))
LINKER_OBJ += $(COMMON_OBJ)

LINKER_DEP = $(patsubst $(LINKER_DIR)/%.cpp, $(OBJ_DIR)/%.d, $(LINKER_SRCS))

CXX = g++ -std=c++17
CXXFLAGS = -MMD -MP -I$(INC_DIR)

all: assembler linker

assembler: $(ASM_OBJ)
	$(CXX) -o $@ $^

linker: $(LINKER_OBJ)
	$(CXX) -o $@ $^

$(OBJ_DIR)/%.o: $(ASM_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(OBJ_DIR)/%.o: $(LINKER_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(OBJ_DIR)/%.o: $(MISC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(OBJ_DIR)/%.o: $(COMMON_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(OBJ_DIR):
	mkdir -p $@

-include $(ASM_DEP)
-include $(LINKER_DEP)
-include $(MISC_DEP)
-include $(COMMON_DEP)

$(BISON_OUTPUT): $(BISON_INPUT)
	bison -d $^

$(FLEX_OUTPUT): $(FLEX_INPUT) 
	flex $^

clean: 
	rm -rf assembler linker
	rm -rf $(OBJ_DIR)
	rm -f $(MISC_DIR)/*.hpp $(MISC_DIR)/*.cpp
	find . -type f \( -name "*.o" -o -name "*.hex" -o -name "*.objdump" \) -delete