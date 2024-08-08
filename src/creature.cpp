#define creature_header
#include "../include/creature.h"

//constructor and destructor
Creature::Creature(std::vector<Action> dna, uint x, uint y, int energy, uint32_t color):
    dna(dna), x(x), y(y), energy(energy), color(color) {
    alive = true;
    dnaAdaptor = dna.begin();
}