CXX = g++
CXXFLAGS = -flto -lm -Ofast --std=c++20

# Directories
SRC_DIR := src
BUILD_DIR := build
OBJ_DIR := build/obj
TARGET := $(BUILD_DIR)/main


SRCS_TMP = main.cpp entities.cpp problem.cpp problem_helper.cpp solution.cpp \
       iterable.cpp solution_data.cpp evaluation.cpp move_generator.cpp \
       move.cpp utils.cpp evaluation_element.cpp perturbator.cpp algorithms.cpp 

SRCS := $(addprefix $(SRC_DIR)/, $(SRCS_TMP))
OBJS := $(addprefix $(OBJ_DIR)/, $(SRCS_TMP:.cpp=.o))


all: $(TARGET)


fo:
	echo $(OBJ_DIR)


# Link the final executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile .cpp to .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create the object directory if it does not exist
$(OBJ_DIR):
	mkdir -p $@



clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean