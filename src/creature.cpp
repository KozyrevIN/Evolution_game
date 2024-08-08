#define creature_header
#include "../include/creature.h"

//constructor and destructor
Creature::Creature(std::vector<Action> dna, uint32_t color, int energy, uint x, uint y, uint8_t direction, Eigen::Vector2<uint> chunk_idx):
    dna(dna), color(color), energy(energy), x(x), y(y), direction(direction), chunkIdx(chunk_idx) {
    alive = true;
    dnaAdaptor = dna.begin();
}