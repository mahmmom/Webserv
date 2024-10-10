CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98
SRC_DIRS := Parser Config
SRCS := $(wildcard main.cpp $(foreach dir, $(SRC_DIRS), $(dir)/*.cpp))
OBJS := $(patsubst %.cpp, objects/%.o, $(SRCS))
TARGET := my_program

# Create the 'objects' directory if it doesn't exist
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

# Compile each .cpp file into the corresponding object file in 'objects' directory
objects/%.o: %.cpp | objects
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Handle main.cpp specifically
objects/main.o: main.cpp | objects
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create the 'objects' directory if it doesn't exist
objects:
	mkdir -p objects/Parser objects/Config objects

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(TARGET)

re: fclean all

.PHONY: all clean fclean re
