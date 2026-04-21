#include "LGPEngine.h"
#include "ISA.h"
#include "LGPConfig.h"

LGPEngine::init(uint32_t seed){
    std::uniform_int_distribution<uint32_t> dist_32(0,UINT32_MAX); // setting up disttribution 
    // sets all instructions for all programs
    for (int i = 0; i < LGPConfig::TOTAL_INSTRUCTIONS; i ++){
        uint32_t raw_rand = dist_32(rng);
        uint32_t encoded_instruction = ISA::encode_from_random(raw_rand);
        data.instructions[i] = encoded_instruction;
    }

} // MUST BE TESTED TOMORROW