#include <vector>
#include <eigen3/Eigen/Dense>

using uint = unsigned int;

//list of actions which a creature can make
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
    reproduce,
    attack
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
    uint8_t prev;

    //constructor
    Creature(const std::vector<Action>& dna, uint32_t color, int energy, uint8_t direction, uint x, uint y);
};