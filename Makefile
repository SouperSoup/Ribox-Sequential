# ---- Configuration ----
CXX      := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -O2
DEPFLAGS := -MMD -MP

# ---- Files ----
CORE_SRCS := LGPEngine.cpp Interpreter.cpp Evaluator.cpp Dataset.cpp
CORE_OBJS := $(CORE_SRCS:.cpp=.o)

# Each binary has its own entry-point translation unit.
TEST_OBJS := $(CORE_OBJS) test_bed.o
RUN_OBJS  := $(CORE_OBJS) main.o
GEN_OBJS  := Dataset.o gen_datasets.o      # ← only needs Dataset

# All objects we might build, for dependency includes and clean.
ALL_OBJS := $(CORE_OBJS) test_bed.o main.o gen_datasets.o   # ← add gen
DEPS     := $(ALL_OBJS:.o=.d)

TEST_TARGET := lgp_test
RUN_TARGET  := lgp_run
GEN_TARGET  := gen_datasets                 # ← new binary

# ---- Targets ----
all: $(TEST_TARGET) $(RUN_TARGET) $(GEN_TARGET)   # ← build it by default

$(TEST_TARGET): $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) $(TEST_OBJS) -o $(TEST_TARGET)

$(RUN_TARGET): $(RUN_OBJS)
	$(CXX) $(CXXFLAGS) $(RUN_OBJS) -o $(RUN_TARGET)

$(GEN_TARGET): $(GEN_OBJS)                  # ← link rule, mirrors the others
	$(CXX) $(CXXFLAGS) $(GEN_OBJS) -o $(GEN_TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

clean:
	rm -f $(ALL_OBJS) $(DEPS) $(TEST_TARGET) $(RUN_TARGET) $(GEN_TARGET)  # ← clean it

-include $(DEPS)

.PHONY: all clean