#ifndef LGP_ENGINE_H
#define LGP_ENGINE_H
#include "LGPConfig.h"
#include <cstdint>
#include <random>
#include <vector>
#include <limits>
struct PopulationData{
    std::vector<uint32_t> instructions;
    std::vector<uint8_t> program_lengths; // assumes programs less then 255

    std::vector<uint32_t> next_gen_instructions; // Write children here during variation
    std::vector<uint8_t>next_gen_lengths;       // Child lengths
    
    std::vector<float> fitness_scores;
    PopulationData()
        : instructions(LGPConfig::TOTAL_INSTRUCTIONS, 0),
          program_lengths(LGPConfig::POPULATION_SIZE, LGPConfig::STARTING_PROGRAM_SIZE),
          next_gen_instructions(LGPConfig::TOTAL_INSTRUCTIONS, 0),
          next_gen_lengths(LGPConfig::POPULATION_SIZE, 0),
          fitness_scores(LGPConfig::POPULATION_SIZE, std::numeric_limits<float>::quiet_NaN()); // any comparison with quiet NAn returns false
    {}
};
class LGPEngine{
    private: 
        
        int current_generation;
        int current_buffer; // flag for which instruct buffer 
	    std::mt19937 rng; 
        std::uniform_int_distribution<uint32_t> dist_32;
        PopulationData data;


    public: 
        LGPEngine();
        ~LGPEngine();
       	void init();
	    void mutate();
        void crossover();
        int tournament_selection(); // returns the program index that is selected
        void vary();// crossover + mutation 
        void evolve(); // evolutionary loop 
    private:
    uint32_t generate_instruction();


};


    

#endif //LGP_ENGINE_H
