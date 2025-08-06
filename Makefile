CXX = g++
CXXFLAGS = -flto -lm -Ofast --std=c++20
TARGET = main

SRCS = $(TARGET).cpp main.cpp entities.cpp problem.cpp problem_helper.cpp solution.cpp \
       iterable.cpp solution_data.cpp evaluation.cpp move_generator.cpp \
       move.cpp utils.cpp evaluation_element.cpp perturbator.cpp algorithms.cpp 
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean