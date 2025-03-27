CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g

# Source files
SRCS = main.cpp processor.cpp no_forward_processor.cpp	ds.cpp
OBJS = $(SRCS:.cpp=.o)
EXEC = a.out

# Default target
all: $(EXEC)

# Linking
$(EXEC): $(OBJS)
	@$(CXX)	$(CXXFLAGS)	-o	$@	$^

# Compilation
%.o: %.cpp
	@$(CXX)	$(CXXFLAGS)	-c	$<	-o	$@

# Run the no-forwarding processor
run: $(EXEC)
	@./$(EXEC)

# Clean build artifacts
clean:
	@rm -f $(OBJS) $(EXEC)

.PHONY: all run clean