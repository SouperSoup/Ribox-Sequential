#include "LGPEngine.h"
#include "LGPConfig.h"
#include "ISA.h"
#include <iostream>
#include <cassert>
#include <cmath>
#include "Interpreter.h"

void print_instruction(uint32_t instr) {
    std::cout << "op=" << (int)ISA::get_op(instr)
              << " dest=r" << (int)ISA::get_dest_index(instr)
              << " src1=r" << (int)ISA::get_src1_index(instr)
              << " src2=" << (ISA::is_src2_constant(instr) ? "C[" : "r[")
              << (int)ISA::get_src2_index(instr) << "]"
              << "\n";
}
void test_interpreter_doubling() {
    uint32_t prog[1] = {
        ISA::encode_manual(ISA::ADD, 0, 0, 0, false)
    };

    float inputs[LGPConfig::NUM_CONTEXTS];
    float outputs[LGPConfig::NUM_CONTEXTS];
    for (int c = 0; c < LGPConfig::NUM_CONTEXTS; ++c) {
        inputs[c] = static_cast<float>(c);
    }

    Interpreter::run_stateless(prog, 1, inputs, 1, outputs);

    for (int c = 0; c < LGPConfig::NUM_CONTEXTS; ++c) {
        float expected = 2.0f * c;
        assert(outputs[c] == expected);
    }

    std::cout << "  test_interpreter_doubling passed\n";
}

void test_interpreter_zero_init() {
    // r0 = r0 + r1, where r1 is scratch (should be 0)
    uint32_t prog[1] = {
        ISA::encode_manual(ISA::ADD, 0, 0, 1, false)
    };

    float inputs[LGPConfig::NUM_CONTEXTS];
    float outputs[LGPConfig::NUM_CONTEXTS];
    for (int c = 0; c < LGPConfig::NUM_CONTEXTS; ++c) {
        inputs[c] = static_cast<float>(c) + 1.0f;  // nonzero inputs
    }

    Interpreter::run_stateless(prog, 1, inputs, 1, outputs);

    // r1 was zero, so output should equal input
    for (int c = 0; c < LGPConfig::NUM_CONTEXTS; ++c) {
        assert(outputs[c] == inputs[c]);
    }

    std::cout << "  test_interpreter_zero_init passed\n";
}
void test_interpreter_constant_access() {
    // r0 = r0 + C[1]   where C[1] = 1.0
    uint32_t prog[1] = {
        ISA::encode_manual(ISA::ADD, 0, 0, 1, /*is_constant=*/true)
    };

    float inputs[LGPConfig::NUM_CONTEXTS];
    float outputs[LGPConfig::NUM_CONTEXTS];
    for (int c = 0; c < LGPConfig::NUM_CONTEXTS; ++c) {
        inputs[c] = static_cast<float>(c);
    }

    Interpreter::run_stateless(prog, 1, inputs, 1, outputs);

    for (int c = 0; c < LGPConfig::NUM_CONTEXTS; ++c) {
        float expected = static_cast<float>(c) + 1.0f;
        assert(outputs[c] == expected);
    }

    std::cout << "  test_interpreter_constant_access passed\n";
}
void test_interpreter_subtract() {
    // r0 = r0 - C[1]    (C[1] = 1.0, so output = input - 1)
    uint32_t prog[1] = {
        ISA::encode_manual(ISA::SUB, 0, 0, 1, true)
    };
    float inputs[LGPConfig::NUM_CONTEXTS], outputs[LGPConfig::NUM_CONTEXTS];
    for (int c = 0; c < LGPConfig::NUM_CONTEXTS; ++c) inputs[c] = static_cast<float>(c);

    Interpreter::run_stateless(prog, 1, inputs, 1, outputs);

    for (int c = 0; c < LGPConfig::NUM_CONTEXTS; ++c) {
        assert(outputs[c] == static_cast<float>(c) - 1.0f);
    }
    std::cout << "  test_interpreter_subtract passed\n";
}

void test_interpreter_multiply() {
    // r0 = r0 * C[5]    (C[5] = 2.0)
    uint32_t prog[1] = {
        ISA::encode_manual(ISA::MUL, 0, 0, 5, true)
    };
    float inputs[LGPConfig::NUM_CONTEXTS], outputs[LGPConfig::NUM_CONTEXTS];
    for (int c = 0; c < LGPConfig::NUM_CONTEXTS; ++c) inputs[c] = static_cast<float>(c);

    Interpreter::run_stateless(prog, 1, inputs, 1, outputs);

    for (int c = 0; c < LGPConfig::NUM_CONTEXTS; ++c) {
        assert(outputs[c] == static_cast<float>(c) * 2.0f);
    }
    std::cout << "  test_interpreter_multiply passed\n";
}
void test_interpreter_protected_div() {
    // r0 = r0 / r1, where r1 is 0 (scratch).
    // Protected div returns 1.0 on zero divisor.
    uint32_t prog[1] = {
        ISA::encode_manual(ISA::DIV, 0, 0, 1, false)
    };
    float inputs[LGPConfig::NUM_CONTEXTS], outputs[LGPConfig::NUM_CONTEXTS];
    for (int c = 0; c < LGPConfig::NUM_CONTEXTS; ++c) {
        inputs[c] = static_cast<float>(c) + 1.0f;  // any nonzero input
    }

    Interpreter::run_stateless(prog, 1, inputs, 1, outputs);

    for (int c = 0; c < LGPConfig::NUM_CONTEXTS; ++c) {
        assert(outputs[c] == 1.0f);  // protected-div fallback
        assert(!std::isnan(outputs[c]));
        assert(!std::isinf(outputs[c]));
    }
    std::cout << "  test_interpreter_protected_div passed\n";
}
void test_interpreter_normal_div() {
    // r0 = r0 / C[5]   (C[5] = 2.0)
    uint32_t prog[1] = {
        ISA::encode_manual(ISA::DIV, 0, 0, 5, true)
    };
    float inputs[LGPConfig::NUM_CONTEXTS], outputs[LGPConfig::NUM_CONTEXTS];
    for (int c = 0; c < LGPConfig::NUM_CONTEXTS; ++c) inputs[c] = static_cast<float>(c) * 2.0f;

    Interpreter::run_stateless(prog, 1, inputs, 1, outputs);

    for (int c = 0; c < LGPConfig::NUM_CONTEXTS; ++c) {
        assert(outputs[c] == static_cast<float>(c));
    }
    std::cout << "  test_interpreter_normal_div passed\n";
}
void test_interpreter_lt() {
    // r0 = r0 < C[5]   (C[5] = 2.0)
    // Inputs 0, 1, 2, 3, ... should give 1, 1, 0, 0, ...
    uint32_t prog[1] = {
        ISA::encode_manual(ISA::LT, 0, 0, 5, true)
    };
    float inputs[LGPConfig::NUM_CONTEXTS], outputs[LGPConfig::NUM_CONTEXTS];
    for (int c = 0; c < LGPConfig::NUM_CONTEXTS; ++c) inputs[c] = static_cast<float>(c);

    Interpreter::run_stateless(prog, 1, inputs, 1, outputs);

    for (int c = 0; c < LGPConfig::NUM_CONTEXTS; ++c) {
        float expected = (static_cast<float>(c) < 2.0f) ? 1.0f : 0.0f;
        assert(outputs[c] == expected);
    }
    std::cout << "  test_interpreter_lt passed\n";
}
void test_interpreter_gt() {
    // r0 = r0 < C[5]   (C[5] = 2.0)
    // Inputs 0, 1, 2, 3, ... should give 1, 1, 0, 0, ...
    uint32_t prog[1] = {
        ISA::encode_manual(ISA::GT, 0, 0, 5, true)
    };
    float inputs[LGPConfig::NUM_CONTEXTS], outputs[LGPConfig::NUM_CONTEXTS];
    for (int c = 0; c < LGPConfig::NUM_CONTEXTS; ++c) inputs[c] = static_cast<float>(c);

    Interpreter::run_stateless(prog, 1, inputs, 1, outputs);

    for (int c = 0; c < LGPConfig::NUM_CONTEXTS; ++c) {
        float expected = (static_cast<float>(c) > 2.0f) ? 1.0f : 0.0f;
        assert(outputs[c] == expected);
    }
    std::cout << "  test_interpreter_lt passed\n";
}
void test_interpreter_sin() {
    // r0 = sin(r0)
    uint32_t prog[1] = {
        ISA::encode_manual(ISA::SIN, 0, 0, 0, false)  // src2 ignored for unary
    };
    float inputs[LGPConfig::NUM_CONTEXTS], outputs[LGPConfig::NUM_CONTEXTS];
    for (int c = 0; c < LGPConfig::NUM_CONTEXTS; ++c) inputs[c] = static_cast<float>(c);

    Interpreter::run_stateless(prog, 1, inputs, 1, outputs);

    const float eps = 1e-5f;
    for (int c = 0; c < LGPConfig::NUM_CONTEXTS; ++c) {
        float expected = std::sin(static_cast<float>(c));
        assert(std::fabs(outputs[c] - expected) < eps);
    }
    std::cout << "  test_interpreter_sin passed\n";
}
void test_interpreter_multi_instruction() {
    // Program:
    //   r1 = r0 + C[1]    (r1 = x + 1)
    //   r0 = r1 * C[5]    (r0 = (x + 1) * 2)
    // For input x, output should be 2x + 2.
    uint32_t prog[2] = {
        ISA::encode_manual(ISA::ADD, 1, 0, 1, true),
        ISA::encode_manual(ISA::MUL, 0, 1, 5, true),
    };

    float inputs[LGPConfig::NUM_CONTEXTS], outputs[LGPConfig::NUM_CONTEXTS];
    for (int c = 0; c < LGPConfig::NUM_CONTEXTS; ++c) inputs[c] = static_cast<float>(c);

    Interpreter::run_stateless(prog, 2, inputs, 1, outputs);

    for (int c = 0; c < LGPConfig::NUM_CONTEXTS; ++c) {
        float expected = 2.0f * static_cast<float>(c) + 2.0f;
        assert(outputs[c] == expected);
    }
    std::cout << "  test_interpreter_multi_instruction passed\n";
}
void test_interpreter_context_independence() {
    // r0 = r0 * r0    (each context squares its own input)
    uint32_t prog[1] = {
        ISA::encode_manual(ISA::MUL, 0, 0, 0, false)
    };

    float inputs[LGPConfig::NUM_CONTEXTS], outputs[LGPConfig::NUM_CONTEXTS];
    for (int c = 0; c < LGPConfig::NUM_CONTEXTS; ++c) inputs[c] = static_cast<float>(c);

    Interpreter::run_stateless(prog, 1, inputs, 1, outputs);

    for (int c = 0; c < LGPConfig::NUM_CONTEXTS; ++c) {
        float expected = static_cast<float>(c) * static_cast<float>(c);
        assert(outputs[c] == expected);
    }
    std::cout << "  test_interpreter_context_independence passed\n";
}
void test_interpreter_determinism() {
    uint32_t prog[1] = { ISA::encode_manual(ISA::ADD, 0, 0, 0, false) };
    float inputs[LGPConfig::NUM_CONTEXTS], out_a[LGPConfig::NUM_CONTEXTS], out_b[LGPConfig::NUM_CONTEXTS];
    for (int c = 0; c < LGPConfig::NUM_CONTEXTS; ++c) inputs[c] = static_cast<float>(c);

    Interpreter::run_stateless(prog, 1, inputs, 1, out_a);
    Interpreter::run_stateless(prog, 1, inputs, 1, out_b);

    for (int c = 0; c < LGPConfig::NUM_CONTEXTS; ++c) {
        assert(out_a[c] == out_b[c]);
    }
    std::cout << "  test_interpreter_determinism passed\n";
}
void test_interpreter_zero_length() {
    uint32_t prog[1] = { 0 };  // never executed since length is 0
    float inputs[LGPConfig::NUM_CONTEXTS], outputs[LGPConfig::NUM_CONTEXTS];
    for (int c = 0; c < LGPConfig::NUM_CONTEXTS; ++c) inputs[c] = static_cast<float>(c) + 1.0f;

    Interpreter::run_stateless(prog, 0, inputs, 1, outputs);

    for (int c = 0; c < LGPConfig::NUM_CONTEXTS; ++c) {
        assert(outputs[c] == inputs[c]);
    }
    std::cout << "  test_interpreter_zero_length passed\n";
}
int main() {
    LGPEngine engine;
    engine.init_population();

    const PopulationData& data = engine.get_data();

    // Structural invariants
    assert(data.instructions.size()    == LGPConfig::TOTAL_INSTRUCTIONS);
    assert(data.program_lengths.size() == LGPConfig::POPULATION_SIZE);
    assert(data.fitness_scores.size()  == LGPConfig::POPULATION_SIZE);

    // Every program should have the starting length
    for (auto len : data.program_lengths) {
        assert(len == LGPConfig::STARTING_PROGRAM_SIZE);
    }

    // Fitness should still be NaN sentinel (we haven't evaluated)
    for (auto f : data.fitness_scores) {
        assert(std::isnan(f));
    }

    // Every decoded instruction in the active range should have valid fields
    for (int p = 0; p < LGPConfig::POPULATION_SIZE; ++p) {
        int base = p * LGPConfig::MAX_PROGRAM_SIZE;

        for (int i = 0; i < data.program_lengths[p]; ++i) {
            uint32_t instr = data.instructions[base + i]; // to access each component 
            assert(ISA::get_op(instr)         < LGPConfig::NUM_OPERATIONS);
            assert(ISA::get_dest_index(instr) < LGPConfig::NUM_REGISTERS);
            assert(ISA::get_src1_index(instr) < LGPConfig::NUM_REGISTERS);
            assert(ISA::get_src2_index(instr) < LGPConfig::NUM_REGISTERS);
        }
    }

    // Eyeball test: print program 0
    std::cout << "Program 0:\n";
    int base = 0;
    for (int i = 0; i < data.program_lengths[0]; ++i) {
        std::cout << "  [" << i << "] ";
        print_instruction(data.instructions[base + i]);
    }
    test_interpreter_doubling();
    test_interpreter_zero_init();
    test_interpreter_constant_access();
    test_interpreter_multiply();
    test_interpreter_subtract();
    test_interpreter_protected_div();
    test_interpreter_normal_div();
    test_interpreter_gt();
    test_interpreter_lt();
    test_interpreter_sin();
    test_interpreter_multi_instruction();
    test_interpreter_context_independence();
    test_interpreter_determinism();
    test_interpreter_zero_length();

    std::cout << "All assertions passed.\n";
    return 0;
}