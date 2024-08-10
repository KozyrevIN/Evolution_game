#define creature_header
#include "../include/creature.h"

//constructor and destructor
Creature::Creature(const std::vector<Action>& dna, uint32_t color, int energy, uint8_t direction, uint x, uint y, std::pair<uint, uint> chunk_id):
    dna(dna), color(color), energy(energy), x(x), y(y), direction(direction), chunkId(chunk_id) {
    prev = true;
    alive = true;
    dnaAdapter = 0;
}