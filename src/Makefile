CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g

# Common source files
COMMON_SRCS = processor.cpp

# No-forwarding processor
NOFORWARD_SRCS = main_no_forward.cpp no_forward_processor.cpp $(COMMON_SRCS)
NOFORWARD_OBJS = $(NOFORWARD_SRCS:.cpp=.o)
NOFORWARD_EXEC = noforward

# Forwarding processor
FORWARD_SRCS = main_forward.cpp forward_processor.cpp $(COMMON_SRCS)
FORWARD_OBJS = $(FORWARD_SRCS:.cpp=.o)
FORWARD_EXEC = forward

# Default target
all: $(NOFORWARD_EXEC) $(FORWARD_EXEC)

# Linking for no-forwarding processor
$(NOFORWARD_EXEC): $(NOFORWARD_OBJS)
	@$(CXX) $(CXXFLAGS) -o $@ $^

# Linking for forwarding processor
$(FORWARD_EXEC): $(FORWARD_OBJS)
	@$(CXX) $(CXXFLAGS) -o $@ $^

# Compilation
%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# Run the no-forwarding processor
run-noforward: $(NOFORWARD_EXEC)
	@./$(NOFORWARD_EXEC)

# Run the forwarding processor
run-forward: $(FORWARD_EXEC)
	@./$(FORWARD_EXEC)

# Clean build artifacts
clean:
	@rm -f $(NOFORWARD_OBJS) $(FORWARD_OBJS) $(NOFORWARD_EXEC) $(FORWARD_EXEC)

.PHONY: all run-noforward run-forward clean