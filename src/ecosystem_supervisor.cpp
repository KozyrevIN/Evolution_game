#include <array>
#include <functional>
#include <iostream>
#include <algorithm>
#include <omp.h>

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
    std::cout << (uint)action << " ";
    while ((action < 8 || !(*it).prev) && dna_adaptor != (*it).dna.end()) {
        actions[action](it);
        action = static_cast<uint8_t>(*++dna_adaptor);
    }

    if ((*it).energy-- <= 0) {
        kill(*it);
    }
}

void EcosystemSupervisor::processChunk(uint chunk_idx_1) {
    for (auto it = creatures[chunk_idx_1].begin(); it != creatures[chunk_idx_1].end();) {
        if ((*it).alive) {
            readDna(it);
            ++it;
        } else {
            auto erease_it = it;
            ++it;
            creatures[chunk_idx_1].erase(erease_it);
        }
    }
} 

void EcosystemSupervisor::checkLost(uint x, uint y, uint chunk_idx_1) {
    if (ecosystem.cells(x, y) != nullptr) {
        auto creature = *(ecosystem.cells(x, y));
        if (creature.chunkIdx[0] != id || creature.chunkIdx[1] != chunk_idx_1) {
            creature.chunkIdx[0] = id;
            creature.chunkIdx[1] = chunk_idx_1;
            creatures[chunk_idx_1].push_front(creature);
        }
    }
}
    
void EcosystemSupervisor::manageLostOnes() {
    for (uint x_pos : {0, 1}) {
        for (uint y_pos : {0, 1}) {
            uint chunk_idx_1 = x_pos + 2 * y_pos;

            uint i = xOrigin + xSize / 2 * x_pos;
            uint j = yOrigin + ySize / 2 * y_pos;
            for (; i < xOrigin + xSize / 2 + (xSize - xSize / 2) * x_pos; ++i) {
                checkLost(i, j, chunk_idx_1);
            }
            --i;
            for (; j < yOrigin + ySize / 2 + (ySize - ySize / 2) * y_pos; ++j) {
                checkLost(i, j, chunk_idx_1);
            }
            --j;
            for (; i > xOrigin + xSize / 2 * x_pos; --i) {
                checkLost(i, j, chunk_idx_1);
            }
            checkLost(i, j, chunk_idx_1);
            for (; j > yOrigin + ySize / 2 * y_pos; --j) {
                checkLost(i, j, chunk_idx_1);
            }
            checkLost(i, j, chunk_idx_1);
        }
    }
}

void EcosystemSupervisor::renderChunks() {
    for (uint chunk_idx_1 = 0; chunk_idx_1 < 4; ++chunk_idx_1) {
        for (auto it = creatures[chunk_idx_1].begin(); it != creatures[chunk_idx_1].end(); ++it) {
            if ((*it).alive) {
                ecosystem.renderCreature(*it);
            }
        }
    }
}

EcosystemSupervisor::EcosystemSupervisor(Ecosystem& ecosystem, uint thread_rows, uint thread_cols, uint seed):
    ecosystem(ecosystem), randGen(seed + id) {
    uint threads = thread_rows * thread_cols; 
    id = omp_get_thread_num();

    xSize = ecosystem.cells.rows() / thread_rows;
    ySize = ecosystem.cells.cols() / thread_cols;

    xOrigin = xSize * (id / thread_rows);
    yOrigin = ySize * (id % thread_rows);

    creatures = std::vector<std::list<Creature>>(4, std::list<Creature>());
}