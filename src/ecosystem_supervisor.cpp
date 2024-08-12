#include <array>
#include <functional>
#include <iostream>
#include <algorithm>
#include <omp.h>
#include <string>

#define ecosystem_supervisor_header
#include "../include/ecosystem_supervisor.h"

//chunk transition mechanism
inline std::pair<uint, uint> EcosystemSupervisor::getChunkId(uint x, uint y) {
    uint x_id = std::min(x / xSize, threadsX - 1);
    uint y_id = std::min(y / ySize, threadsY - 1);
 
    uint x_from_origin = x - x_id * xSize;
    uint y_from_origin = y - y_id * ySize;

    uint chunk_id_0 = y_id * threadsX + x_id;
    uint chunk_id_1 = (y_from_origin >= ySize / 2) * 2 + (x_from_origin >= xSize / 2);

    assert(chunk_id_0 >= 0 && chunk_id_0 < threadsX * threadsY && chunk_id_1 >= 0 && chunk_id_1 <= 3);

    return std::make_pair(chunk_id_0, chunk_id_1);
}

inline void EcosystemSupervisor::moveToInternalBuffer(std::list<Creature>::iterator& it, uint to_id_1, uint from_id_1) {
    assert((*it).chunkId.first == id && to_id_1 >= 0 && to_id_1 <= 3 && from_id_1 >= 0 && from_id_1 <= 3);

    auto erease_it = it++;
    internalExchangeBuffer[to_id_1].splice(internalExchangeBuffer[to_id_1].begin(), creatures[from_id_1], erease_it);
}

inline void EcosystemSupervisor::moveToExternalBuffer(std::list<Creature>::iterator& it, uint to_id_0, uint from_id_1) {
    assert(to_id_0 >= 0 && to_id_0 <= threadsX * threadsY);

    auto erease_it = it++;
    externalExchangeBuffer(to_id_0, id).push_front(*erease_it);
    creatures[from_id_1].erase(erease_it);
    auto new_it = externalExchangeBuffer(to_id_0, id).begin();
    ecosystem.cells((*new_it).x, (*new_it).y) = &(*new_it);
}

inline void EcosystemSupervisor::emptyInternalBuffer(uint to_id_1) {
    creatures[to_id_1].splice(creatures[to_id_1].begin(), internalExchangeBuffer[to_id_1]);
}

inline void EcosystemSupervisor::emptyExternalBuffer(uint from_id_0) {
    while (!externalExchangeBuffer(id, from_id_0).empty()) {
        auto it = externalExchangeBuffer(id, from_id_0).begin();
        uint x = (*it).x;
        uint y = (*it).y;
        auto to_id = getChunkId(x, y);
        creatures[to_id.second].push_front(*it);
        ecosystem.cells(x, y) = &(*creatures[to_id.second].begin());
        externalExchangeBuffer(id, from_id_0).pop_front();
    }
}

inline void EcosystemSupervisor::checkAndHandleChunkTransition(std::list<Creature>::iterator& it, uint from_id_1) {
    auto old_chunk_id = std::make_pair(id, from_id_1);
    auto new_chunk_id = getChunkId((*it).x, (*it).y);

    if (new_chunk_id == old_chunk_id) {
        ++it;
    } else if (new_chunk_id.first != old_chunk_id.first) {
        moveToExternalBuffer(it, new_chunk_id.first, old_chunk_id.second);
    } else if (new_chunk_id.second < old_chunk_id.second) {
        auto erease_it = it++;
        creatures[new_chunk_id.second].splice(creatures[new_chunk_id.second].begin(), creatures[old_chunk_id.second], erease_it);
    } else if (new_chunk_id.second > old_chunk_id.second) {
        moveToInternalBuffer(it, new_chunk_id.second, old_chunk_id.second);
    }
}

//helper functions
inline void EcosystemSupervisor::kill(Creature& creature) {
    ecosystem.cells(creature.x, creature.y) = nullptr;
    creature.alive = false;
}

inline uint32_t dna_hash(const std::vector<Action>& dna) {
    const uint8_t num_genes = 13;
    const uint8_t renorm_value = 128 / num_genes;
    Eigen::Array3<uint8_t> color_components = {0, 0, 0};

    for (uint i = 0; i < dna.size(); i++) {
        color_components(i % 3) += static_cast<uint8_t>(dna[i]) * renorm_value;
    }

    uint sum = color_components.sum();
    if (sum < 0xFF) {
        color_components += 1;
        color_components *= (0xFF / (sum + 3)) + 1;
    }

    return 0xFF000000 + 0x00010000 * color_components[0] + 0x00000100 * color_components[1] + 0x00000001 * color_components[2];
}

inline std::pair<std::vector<Action>, uint32_t> EcosystemSupervisor::mutate(const std::vector<Action>& dna, uint32_t color) {
    const uint8_t num_genes = 13;
    const uint max_length = 0xFFF;

    uint random = randGen.next();

    if (random % 2 == 0) {
        random /= 2;
        std::vector<Action> new_dna = dna;

        auto it = new_dna.begin();
        uint pos = random % new_dna.size();
        random /= max_length;
        std::advance(it, pos);

        if (random % 8 == 0 && new_dna.size() < max_length) {
            //adds new gene to dna
            new_dna.insert(it, static_cast<Action>(random % num_genes));
            color = dna_hash(new_dna); 
        } else if (random % 8 == 1) {
            //removes gene from dna
            new_dna.erase(it);
            color = dna_hash(new_dna); 
        } else if (random % 8 == 2) {
            //changes gene in dna to random
            *it = static_cast<Action>(random % num_genes);
        }

        return std::make_pair(new_dna, color);
    } else {
        return std::make_pair(dna, color);
    }
}

inline void EcosystemSupervisor::placeNewborn(const std::vector<Action>& parent_dna, const uint32_t parent_color, int energy,
                                              uint x, uint y, uint parent_chunk_id_1) {
    auto parent_chunk_id = std::make_pair(id, parent_chunk_id_1);
    auto newborn_chunk_id = getChunkId(x, y);
    auto [newborn_dna, newborn_color] = mutate(parent_dna, parent_color);

    if (newborn_chunk_id.first == parent_chunk_id.first) {
        if (newborn_chunk_id.second <= parent_chunk_id.second) {
            creatures[newborn_chunk_id.second].emplace_front(
                newborn_dna, newborn_color, energy, randGen.next() % 4 + 1, x, y);
            ecosystem.cells(x, y) = &(*creatures[newborn_chunk_id.second].begin());
        } else {
            internalExchangeBuffer[newborn_chunk_id.second].emplace_front(
                newborn_dna, newborn_color, energy, randGen.next() % 4 + 1, x, y);
            ecosystem.cells(x, y) = &(*internalExchangeBuffer[newborn_chunk_id.second].begin());
        }
    } else {
        externalExchangeBuffer(newborn_chunk_id.first, parent_chunk_id.first).emplace_front(
            newborn_dna, newborn_color, energy, randGen.next() % 4 + 1, x, y);
        ecosystem.cells(x, y) = &(*externalExchangeBuffer(newborn_chunk_id.first, parent_chunk_id.first).begin());
    }
}

inline void EcosystemSupervisor::doAction(std::list<Creature>::iterator& it, uint chunk_id_1) {
    bool move_flag = false;
    uint8_t action;
    Creature& creature = *it;

    do {
        switch (action = static_cast<uint8_t>(creature.dna[creature.dnaAdapter++]); action) {
            case 0: //nothing
                creature.prev = true;
                break;
            case 1: //negation
                creature.prev = !creature.prev;
                break;
            case 2: //comp_1
                creature.prev = creature.prev >= 1;
                break;
            case 3: //add_1
                creature.prev++;
                break;
            case 4: //mul_2
                creature.prev *= 2;
                break;
            case 5: //sub_1
                creature.prev--;
                break;
            case 6: //div_2
                creature.prev /= 2;
                break;
            case 7: //random
                creature.prev = (creature.prev > 0) * (randGen.next() % 254 + 1);
                break;

            //end of logical actions
            case 8: //rotate
                creature.direction = (creature.direction - 1 + creature.prev) % 4 + 1;
                creature.prev = true;
                break;
            case 9:  //move_forward 
                if (creature.prev) {
                    uint new_x = creature.x + (creature.direction == 1) - (creature.direction == 3);
                    uint new_y = creature.y + (creature.direction == 2) - (creature.direction == 4);

                    if (ecosystem.isEmpty(new_x, new_y)) {
                        std::swap(ecosystem.cells(creature.x, creature.y), ecosystem.cells(new_x, new_y));
                        creature.x = new_x;
                        creature.y = new_y;
                        move_flag = true;
                        creature.prev = true;
                    } else {
                        creature.prev = false;
                    }
                }
                break;
            case 10: //photosynthesize
                if (creature.prev && creature.energy > 0) {
                    creature.energy += ecosystem.countEmptyAdjacent(creature.x, creature.y) - 4;
                    creature.prev = true;
                } else {
                    creature.prev = false;
                }
                break;
            case 11: //reproduce
                if (creature.prev && creature.energy > creature.dna.size() + creature.prev) {
                    auto empty_adjacent = ecosystem.emptyAdjacent(creature.x, creature.y);
                    if (!empty_adjacent.empty()) {
                        creature.energy -= creature.dna.size() + creature.prev;
                        auto [new_x, new_y] = empty_adjacent[randGen.next() % empty_adjacent.size()];
                        placeNewborn(creature.dna, creature.color, creature.prev, new_x, new_y, chunk_id_1);
                        creature.prev = true;
                    } else {
                        creature.prev = false;
                    }
                } else {
                    creature.prev = false;
                }
                break;
            case 12: //attack
                if (creature.prev && creature.energy > 2) {
                    uint attack_x = creature.x + (creature.direction == 1) - (creature.direction == 3);
                    uint attack_y = creature.y + (creature.direction == 2) - (creature.direction == 4);

                    if (ecosystem.containsCreature(attack_x, attack_y)) {
                        if(creature.energy > ecosystem.cells(attack_x, attack_y) -> energy) {
                            creature.energy += ecosystem.cells(attack_x, attack_y) -> energy + ecosystem.cells(attack_x, attack_y) -> dna.size();
                            kill(*ecosystem.cells(attack_x, attack_y));
                        }
                    }
                } 
        }
    } while (!(action >= 8 && (creature.prev)) && creature.dnaAdapter < creature.dna.size());

    if (creature.dnaAdapter == creature.dna.size()) {
        creature.dnaAdapter = 0;
    }
    
    creature.energy--;
    if (creature.energy < 0) {
        auto erease_it = it++;
        ecosystem.cells(erease_it -> x, erease_it -> y) = nullptr;
        creatures[chunk_id_1].erase(erease_it);
    } else if (move_flag) {
        checkAndHandleChunkTransition(it, chunk_id_1);
    } else {
        ++it;
    }
}

void EcosystemSupervisor::processChunk(uint chunk_id_1) {
    for (auto it = creatures[chunk_id_1].begin(); it != creatures[chunk_id_1].end();) {
        if ((*it).alive) {
            doAction(it, chunk_id_1);
        } else {
            auto erease_it = it++;
            creatures[chunk_id_1].erase(erease_it);
        }
    }
} 

void EcosystemSupervisor::processExchangeBuffers() {
    for (uint chunk_id_1 = 0; chunk_id_1 < 4; ++chunk_id_1) {
        emptyInternalBuffer(chunk_id_1);
    }

    for (uint chunk_id_0 = 0; chunk_id_0 < threadsX * threadsY; ++chunk_id_0) {
        emptyExternalBuffer(chunk_id_0);
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

void EcosystemSupervisor::addFirstLife(Creature& creature) {
    creature.color = dna_hash(creature.dna);
    auto chunk_id = getChunkId(creature.x, creature.y);
    if (chunk_id.first == id) {
        creatures[chunk_id.second].push_front(creature);
        ecosystem.cells(creature.x, creature.y) = &(*creatures[chunk_id.second].begin());
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