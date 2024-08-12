#include <cassert>

#define creature_header
#include "../include/creature.h"

//constructor and destructor
Creature::Creature(const std::vector<Action>& dna, uint32_t color, int energy, uint8_t direction, uint x, uint y):
                   dna(dna), color(color), energy(energy), x(x), y(y), direction(direction) {

    assert(color > 128 && direction >= 1 && direction <= 4);

    prev = true;
    alive = true;
    dnaAdapter = 0;
}