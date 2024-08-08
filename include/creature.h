#include <vector>
#include <eigen3/Eigen/Dense>

enum class Action { nothing, move_up, move_down, move_right, move_left, photosynthesize, reproduce };

struct Creature {
    //creature properties                                        
    std::vector<Action> dna;
    uint32_t color;

    //creature state
    bool alive;
    int energy;
    uint x;
    uint y;
    std::vector<Action>::iterator dnaAdaptor;

    //creature position in term of supervisors
    Eigen::Vector2<uint> chunk;

    //constructor
    Creature(std::vector<Action> dna, uint x, uint y, int energy, uint32_t color);
};