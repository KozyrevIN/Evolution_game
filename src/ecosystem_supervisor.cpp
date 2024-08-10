#include <array>
#include <functional>
#include <iostream>
#include <algorithm>
#include <omp.h>
#include <string>

#define ecosystem_supervisor_header
#include "../include/ecosystem_supervisor.h"

//chunk transition mechanism
std::pair<uint, uint> EcosystemSupervisor::getChunkId(uint x, uint y) {
    uint x_id = x / xSize;
    uint y_id = y / ySize;
 
    uint x_from_origin = x - x_id * xSize;
    uint y_from_origin = y - y_id * ySize;

    return std::make_pair(y_id * threadsX + x_id, (y_from_origin >= ySize / 2) * 2 + (x_from_origin >= xSize / 2));
}

void EcosystemSupervisor::putToInternalBuffer(std::list<Creature>::iterator& it, uint chunk_id_1) {
    auto erease_it = it++;
    internalExchangeBuffer[chunk_id_1].push_front(*erease_it);
    creatures[(*erease_it).chunkId.second].erase(erease_it);
    auto new_it = internalExchangeBuffer[chunk_id_1].begin();
    (*new_it).chunkId.second = chunk_id_1;
    ecosystem.cells((*new_it).x, (*new_it).y) = &(*new_it);
}

void EcosystemSupervisor::putToExternalBuffer(std::list<Creature>::iterator& it, std::pair<uint, uint> chunk_id) {
    auto erease_it = it++;
    externalExchangeBuffer(chunk_id.first, (*erease_it).chunkId.first).push_front(*erease_it);
    creatures[(*erease_it).chunkId.second].erase(erease_it);
    auto new_it = externalExchangeBuffer(chunk_id.first, (*erease_it).chunkId.first).begin();
    (*new_it).chunkId = chunk_id;
    ecosystem.cells((*new_it).x, (*new_it).y) = &(*new_it);
}

void EcosystemSupervisor::checkChunkBounds(std::list<Creature>::iterator& it) {
    auto new_chunk_id = getChunkId((*it).x, (*it).y);
    if (new_chunk_id == (*it).chunkId) {
        ++it;
    } else if (new_chunk_id.first != (*it).chunkId.first) {
        putToExternalBuffer(it, new_chunk_id);
    } else if (new_chunk_id.second < (*it).chunkId.second) {
        auto erease_it = it++;
        creatures[new_chunk_id.second].push_front(*erease_it);
        creatures[(*erease_it).chunkId.second].erase(erease_it);
        auto new_it = creatures[new_chunk_id.second].begin();
        (*new_it).chunkId = new_chunk_id;
        ecosystem.cells((*new_it).x, (*new_it).y) = &(*new_it);
    } else if (new_chunk_id.second > (*it).chunkId.second) {
        putToInternalBuffer(it, new_chunk_id.second);
    }
}

//helper functions
void EcosystemSupervisor::kill(Creature& creature) {
    ecosystem.cells(creature.x, creature.y) = nullptr;
    creature.alive = false;
}

std::vector<Action> mutate(const std::vector<Action>& dna) {
    return dna;
}

uint32_t mutate(uint32_t color) {
    return color;
}

void EcosystemSupervisor::placeNewborn(const std::vector<Action>& parent_dna, const uint32_t parent_color,
    std::pair<uint, uint> parent_chunk_id, uint x, uint y, int energy) {
    auto newborn_chunk_id = getChunkId(x, y);
    if (newborn_chunk_id.first == parent_chunk_id.first) {
        if (newborn_chunk_id.second <= parent_chunk_id.second) {
            creatures[newborn_chunk_id.second].emplace_front(
                mutate(parent_dna), mutate(parent_color), energy, randGen.next() % 4 + 1, x, y, newborn_chunk_id);
            ecosystem.cells(x, y) = &(*creatures[newborn_chunk_id.second].begin());
        } else {
            internalExchangeBuffer[newborn_chunk_id.second].emplace_front(
                mutate(parent_dna), mutate(parent_color), energy, randGen.next() % 4 + 1, x, y, newborn_chunk_id);
            ecosystem.cells(x, y) = &(*internalExchangeBuffer[newborn_chunk_id.second].begin());
        }
    } else {
        externalExchangeBuffer(newborn_chunk_id.first, parent_chunk_id.first).emplace_front(
            mutate(parent_dna), mutate(parent_color), energy, randGen.next() % 4 + 1, x, y, newborn_chunk_id);
        ecosystem.cells(x, y) = &(*externalExchangeBuffer(newborn_chunk_id.first, parent_chunk_id.first).begin());
    }
}

void EcosystemSupervisor::doAction(std::list<Creature>::iterator& it) {
    bool move_flag = false;

    using action_def = std::function<void(Creature&)>;

    //logical actions
    action_def nothing = [this](Creature& creature) {
        creature.prev = true;
    };

    action_def negation = [this](Creature& creature) {
        creature.prev = !creature.prev;
    };

    action_def comp_1 = [this](Creature& creature) {
        creature.prev = creature.prev > 0;
    };

    action_def add_1 = [this](Creature& creature) {
        ++creature.prev;
    };

    action_def sub_1 = [this](Creature& creature) {
        --creature.prev;
    };

    action_def mul_2 = [this](Creature& creature) {
        creature.prev *= 2;
    };

    action_def div_2 = [this](Creature& creature) {
        creature.prev /= 2;
    };

    action_def random = [this](Creature& creature) {
        creature.prev = (creature.prev > 0) * (randGen.next() % 254 + 1);
    };

    //physical actions
    action_def rotate = [this](Creature& creature) {
        creature.direction = (creature.direction - 1 + creature.prev) % 4 + 1;
        creature.prev = true;
    };

    action_def move_forward = [this, &move_flag](Creature& creature) {
        uint new_x = creature.x + (creature.direction == 1) - (creature.direction == 3);
        uint new_y = creature.y + (creature.direction == 2) - (creature.direction == 4);

        if (ecosystem.isEmpty(new_x, new_y)) {
            std::swap(ecosystem.cells(creature.x, creature.y), ecosystem.cells(new_x, new_y));
            creature.x = new_x;
            creature.y = new_y;
            move_flag = true;
        } else {
            creature.prev = false;
        }
    };

    action_def photosynthesize = [this](Creature& creature) {
        if (creature.energy > 0) {
            creature.energy += ecosystem.countEmptyAdjacent(creature.x, creature.y) - 1;
            creature.prev = true;
        } else {
            creature.prev = false;
        }
    };

    action_def reproduce = [this](Creature& creature) {
        if (creature.energy > creature.dna.size() + creature.prev) {
            auto empty_adjacent = ecosystem.emptyAdjacent(creature.x, creature.y);
            if (!empty_adjacent.empty()) {
                creature.energy -= creature.dna.size() + creature.prev;
                auto [new_x, new_y] = empty_adjacent[randGen.next() % empty_adjacent.size()];
                this -> placeNewborn(creature.dna, creature.color, creature.chunkId, new_x, new_y, creature.prev);
                creature.prev = true;
            } else {
                creature.prev = false;
            }
        } else {
            creature.prev = false;
        }
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

    uint8_t action;

    do {
        action = static_cast<uint8_t>((*it).dna[(*it).dnaAdapter++]);
        actions[action](*it);
    } while ((!(action >= 8 && (*it).prev)) && (*it).dnaAdapter < (*it).dna.size());

    if ((*it).dnaAdapter == (*it).dna.size()) {
        (*it).dnaAdapter = 0;
    }
    
    (*it).energy--;
    if ((*it).energy < 0) {
        auto erease_it = it++;
        ecosystem.cells((*erease_it).x, (*erease_it).y) = nullptr;
        creatures[(*erease_it).chunkId.second].erase(erease_it);
    } else if (move_flag) {
        checkChunkBounds(it);
    } else {
        ++it;
    }
}

void EcosystemSupervisor::processChunk(uint chunk_idx_1) {
    for (auto it = creatures[chunk_idx_1].begin(); it != creatures[chunk_idx_1].end();) {
        if ((*it).alive) {
            doAction(it);
        } else {
            auto erease_it = it++;
            creatures[chunk_idx_1].erase(erease_it);
        }
    }
} 

void EcosystemSupervisor::processExchangeBuffers() {
    for (uint chunk_id_1 = 0; chunk_id_1 < 4; ++chunk_id_1) {
        while (!internalExchangeBuffer[chunk_id_1].empty()) {
            auto it = internalExchangeBuffer[chunk_id_1].begin();
            creatures[chunk_id_1].push_front(*it);
            ecosystem.cells((*it).x, (*it).y) = &(*creatures[chunk_id_1].begin());
            internalExchangeBuffer[chunk_id_1].pop_front();
        }
    }

    for (uint chunk_id_0 = 0; chunk_id_0 < threadsX * threadsY; ++chunk_id_0) {
        while (!externalExchangeBuffer(id, chunk_id_0).empty()) {
            auto it = externalExchangeBuffer(id, chunk_id_0).begin();
            creatures[(*it).chunkId.first].push_front(*it);
            ecosystem.cells((*it).x, (*it).y) = &(*creatures[(*it).chunkId.first].begin());
            externalExchangeBuffer(id, chunk_id_0).pop_front();
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

void EcosystemSupervisor::addFirstLife(const Creature& creature) {
    if (creature.chunkId.first == id) {
        creatures[creature.chunkId.second].push_front(creature);
        ecosystem.cells(creature.x, creature.y) = &(*creatures[creature.chunkId.second].begin());
    }
}

EcosystemSupervisor::EcosystemSupervisor(Ecosystem& ecosystem, Eigen::MatrixX<std::list<Creature>>& externalExchangeBuffer,
    uint threads_x, uint threads_y, uint seed): ecosystem(ecosystem), externalExchangeBuffer(externalExchangeBuffer),
    randGen(seed + id), threadsX(threads_x), threadsY(threads_y) {
    uint threads = threads_x * threads_y; 
    id = omp_get_thread_num();

    xSize = ecosystem.cells.rows() / threads_x;
    ySize = ecosystem.cells.cols() / threads_y;

    xOrigin = xSize * (id % threads_x);
    yOrigin = ySize * (id / threads_x);

    creatures = std::vector<std::list<Creature>>(4, std::list<Creature>());
    internalExchangeBuffer = std::vector<std::list<Creature>>(4, std::list<Creature>());
}