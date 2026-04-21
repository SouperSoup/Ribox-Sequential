#include "LGPEngine.h"
#include "ISA.h"
#include "LGPConfig.h"

LGPEngine::init(){
    // sets all instructions for all programs
    for (int i = 0; i < LGPConfig::TOTAL_INSTRUCTIONS; i ++){
        data.instructions[i] = generate_instruction();
    }
    

} // MUST BE TESTED TOMORROW
uint32_t LGPEngine::generate_instruction(){
    std::uniform_int_distribution<uint32_t> dist_32(0,UINT32_MAX); // setting up disttribution 
    uint32_t raw_rand = dist_32(rng);
    return ISA::encode_from_random(raw_rand);
}
LGPEngine::LGPEngine():current_generation(0),current_buffer(0), rng(LGPConfig::SEED), dist_32(0,UINT32_MAX),  data(){} // constructor- kept explicitly clean