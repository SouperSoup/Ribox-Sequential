# ---- Configuration ----
CXX      := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -O2
DEPFLAGS := -MMD -MP

# ---- Files ----
# Core engine objects shared by both binaries.
CORE_SRCS := LGPEngine.cpp Interpreter.cpp Evaluator.cpp Dataset.cpp
CORE_OBJS := $(CORE_SRCS:.cpp=.o)

# Each binary has its own entry-point translation unit.
TEST_OBJS := $(CORE_OBJS) test_bed.o
RUN_OBJS  := $(CORE_OBJS) main.o

# All objects we might build, for dependency includes and clean.
ALL_OBJS := $(CORE_OBJS) test_bed.o main.o
DEPS     := $(ALL_OBJS:.o=.d)

TEST_TARGET := lgp_test
RUN_TARGET  := lgp_run

# ---- Targets ----
# Default builds both binaries.
all: $(TEST_TARGET) $(RUN_TARGET)

$(TEST_TARGET): $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) $(TEST_OBJS) -o $(TEST_TARGET)

$(RUN_TARGET): $(RUN_OBJS)
	$(CXX) $(CXXFLAGS) $(RUN_OBJS) -o $(RUN_TARGET)

# Pattern rule: compile any .cpp into a .o with header-dependency tracking.
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

clean:
	rm -f $(ALL_OBJS) $(DEPS) $(TEST_TARGET) $(RUN_TARGET)

-include $(DEPS)

.PHONY: all clean