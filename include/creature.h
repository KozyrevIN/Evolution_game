#include <vector>
#include <eigen3/Eigen/Dense>

using uint = unsigned int;

enum class Action: uint8_t { 
    nothing,
    negation,
    comp_1,
    add_1,
    sub_1,
    mul_2,
    div_2,
    random,

    rotate,
    move_forward,
    photosynthesize,
    reproduce
};

struct Creature {
    //creature properties                                        
    std::vector<Action> dna;
    uint32_t color;

    //creature state
    bool alive;
    int energy;
    uint x;
    uint y;
    uint8_t direction;

    //reading dna
    uint dnaAdapter;
    uint prev;

    //creature position in term of supervisors
    std::pair<uint, uint> chunkId;

    //constructor
    Creature(const std::vector<Action>& dna, uint32_t color, int energy, uint8_t direction, uint x, uint y, std::pair<uint, uint> chunk_id);
};