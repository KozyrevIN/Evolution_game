#include <array>
#include <functional>
#include <iostream>
#include <algorithm>

#define ecosystem_supervisor_header
#include "../include/ecosystem_supervisor.h"

Eigen::Vector2<uint> EcosystemSupervisor::getChunkIdx(uint x, uint y) {
    Eigen::Vector2<uint> chunk_idx;
    if (x >= xOrigin && x <= xOrigin + xSize && y >= yOrigin && y <= yOrigin + ySize) {
        chunk_idx(0) = id;
        chunk_idx(1) = (uint)(x >= xOrigin + xSize / 2) + 2 * (uint)(y >= yOrigin + ySize / 2);
    } else {
        chunk_idx(0) = UINT_MAX;
    }
    return chunk_idx;
}

void EcosystemSupervisor::kill(Creature& creature) {
    ecosystem.cells(creature.x, creature.y) = nullptr;
    creature.alive = false;
}

void EcosystemSupervisor::checkChunkBounds(std::list<Creature>::iterator& it) {
    auto chunk_idx = getChunkIdx((*it).x, (*it).y);
    if (chunk_idx != (*it).chunkIdx) {
        creatures[(*it).chunkIdx(1)].erase(it);
    }
}

void EcosystemSupervisor::readDna(std::list<Creature>::iterator& it) {
    using leash = std::list<Creature>::iterator&;
    using action_def = std::function<void(leash)>;

    //logical actions
    action_def nothing = [this](leash it) {
        (*it).prev = 1;
    };

    action_def negation = [this](leash it) {
        (*it).prev = !(*it).prev;
    };

    action_def comp_1 = [this](leash it) {
        (*it).prev = (*it).prev > 0;
    };

    action_def add_1 = [this](leash it) {
        ++(*it).prev;
    };

    action_def sub_1 = [this](leash it) {
        --(*it).prev;
    };

    action_def mul_2 = [this](leash it) {
        (*it).prev *= 2;
    };

    action_def div_2 = [this](leash it) {
        (*it).prev /= 2;
    };

    action_def random = [this](leash it) {
        (*it).prev = ((*it).prev > 0) * (randGen.next() % 254 + 1);
    };

    //physical actions
    action_def rotate = [this](leash it) {
        (*it).direction = ((*it).direction - 1 + (*it).prev) % 4 + 1;
        (*it).prev = true;
    };

    action_def move_forward = [this](leash it) {
        uint new_x = (*it).x + ((*it).direction == 1) - ((*it).direction == 3);
        uint new_y = (*it).y + ((*it).direction == 2) - ((*it).direction == 4);

        if (ecosystem.isEmpty(new_x, new_y)) {
            ecosystem.cells((*it).x, (*it).y) = nullptr;
            (*it).x = new_x;
            (*it).y = new_y;
            ecosystem.cells(new_x, new_y) = &(*it);
            (*it).energy--;
            checkChunkBounds(it);
            (*it).prev = true;
        } else {
            (*it).prev = false;
        }
    };

    action_def photosynthesize = [this](leash it) {
        if ((*it).energy > 0) {
            (*it).energy += ecosystem.countEmptyAdjacent((*it).x, (*it).y) - 1;
            (*it).prev = true;
        } else {
            (*it).prev = false;
        }
    };

    action_def reproduce = [this](leash it) {
        if ((*it).energy > (*it).dna.size() + (*it).prev) {
            auto empty_adjacent = ecosystem.emptyAdjacent((*it).x, (*it).y);
            if (!empty_adjacent.empty()) {
                (*it).energy -= (*it).dna.size() + (*it).prev / 2;
                auto [new_x, new_y] = empty_adjacent[randGen.next() % empty_adjacent.size()];
                Creature new_creature((*it).dna, (*it).color, (*it).prev / 2, new_x, new_y, randGen.next() % 4 + 1, (*it).chunkIdx);
                ecosystem.cells(new_x, new_y) = &new_creature;
                if ((*it).chunkIdx == getChunkIdx(new_x, new_y)) {
                    creatures[(*it).chunkIdx(0)].push_front(new_creature);
                }
                (*it).prev = true;
            }
        }
        (*it).prev = false;
    };

    static const auto actions = std::array<action_def, 12> {
        nothing,
        negation,
        comp_1,
        add_1,
        sub_1,
        mul_2,
        div_2,
        random, //8

        rotate,
        move_forward,
        photosynthesize,
        reproduce
    };

    auto dna_adaptor = (*it).dnaAdaptor;
    if (dna_adaptor == (*it).dna.end()) {
        dna_adaptor = (*it).dna.begin();
    }

    uint8_t action = static_cast<uint8_t>(*dna_adaptor);
    while ((action < 8 || !(*it).prev) && dna_adaptor != (*it).dna.end()) {
        actions[action](it);
        action = static_cast<uint8_t>(*++dna_adaptor);
    }

    if ((*it).energy-- <= 0) {
        kill(*it);
    }
}