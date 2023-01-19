
# generic project variables
EXECUTABLE_NAME := simulator
IMPLEMENTATION_DIR := source
INCLUDE_DIRS := source
BUILD_DIR := build
OUTPUT_DIR := bin

# change compiler stuff here (everything except CXX gets put into CXX_FLAGS)
CXX := g++
LANG_VERSION := -std=c++20
DEV_FLAGS := -Wall -g3 -ggdb
TESTING_FLAGS := #-Wextra -Wconversion -Wunreachable-code -Winline -Wdisabled-optimization
OPTIMIZATION_FLAGS := -Og
DEP_FLAGS := -MP -MD


###############################################################################
OUTPUT_BINARY := $(OUTPUT_DIR)/$(EXECUTABLE_NAME)

C_FILES := $(wildcard $(IMPLEMENTATION_DIR)/*.c)
CPP_FILES := $(wildcard $(IMPLEMENTATION_DIR)/*.cpp)
CXX_FILES := $(C_FILES) $(CPP_FILES)

C_OBJECT_FILES := $(patsubst $(IMPLEMENTATION_DIR)/%.c,$(BUILD_DIR)/%.o, $(C_FILES))
CPP_OBJECT_FILES := $(patsubst $(IMPLEMENTATION_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(CPP_FILES))
OBJECT_FILES := $(C_OBJECT_FILES) $(CPP_OBJECT_FILES)

DEP_FILES := $(patsubst %.o,%.d,$(OBJECT_FILES))
INCLUDE_DIRS := $(foreach DIR,$(INCLUDE_DIRS),-I$(DIR))
CXX_FLAGS := $(LANG_VERSION) $(INCLUDE_DIRS) $(DEP_FLAGS) $(DEV_FLAGS) $(OPTIMIZATION_FLAGS)
###############################################################################


# compile all code
compile: $(OUTPUT_BINARY) clean-deps

# link object files into final binary
$(OUTPUT_BINARY): $(OBJECT_FILES)
	$(CXX) $(CXX_FLAGS) -o $@ $^

# compile C source code into object files
$(BUILD_DIR)/%.o: $(IMPLEMENTATION_DIR)/%.c
	$(CXX) $(CXX_FLAGS) -c -o $@ $<

# compile C++ source code into object files
$(BUILD_DIR)/%.o: $(IMPLEMENTATION_DIR)/%.cpp
	$(CXX) $(CXX_FLAGS) -c -o $@ $<

# delete everything generated from compiling/linking/running
clean: clean-deps
	-@rm -f $(OUTPUT_BINARY) $(OBJECT_FILES) $(DEP_FILES)
	-@rm -f $(BUILD_DIR)/*.o

# delete *.d files
clean-deps:
	-@rm -f $(DEP_FILES)
	-@rm -f $(OUTPUT_DIR)/*.d

# run with default selections
run: compile
	cd ./bin; ./$(EXECUTABLE_NAME)

valgrind-run: compile
	cd ./bin; valgrind ./$(EXECUTABLE_NAME)

# tell Make to use the generated *.d dependency files
-include $(DEP_FILES)
