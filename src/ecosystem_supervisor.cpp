#include <array>
#include <functional>
#include <iostream>
#include <algorithm>

#define ecosystem_supervisor_header
#include "../include/ecosystem_supervisor.h"

void EcosystemSupervisor::kill(std::list<Creature>& creatures, std::list<Creature>::iterator pos) {
    cells((*pos).x, (*pos).y) = nullptr;
    creatures.erase(pos);
}

void Ecosystem::addNew(std::list<Creature>& creatures, Creature& creature) {
    cells(creature.x, creature.y) = &creature;
    creatures.push_front(creature);
}

bool Ecosystem::readDna(Creature& creature) {
    auto moveUp = [this](Creature& creature, bool flag) {
        if (flag && creature.y > 0 && isEmpty(creature.x, creature.y - 1)) {
            cells(creature.x, creature.y) = nullptr;
            creature.y--;
            cells(creature.x, creature.y) = &creature;
            creature.energy--;
            return true;
        }
        return false;
    };

    auto moveDown = [this](Creature& creature, bool flag) {
        if (flag && creature.y < cells.rows() - 1 && isEmpty(creature.x, creature.y + 1)) {
            cells(creature.x, creature.y) = nullptr;
            creature.y++;
            cells(creature.x, creature.y) = &creature;
            creature.energy--;
            return true;
        }
        return false;
    };

    auto moveRight = [this](Creature& creature, bool flag) {
        if (flag && creature.x < cells.cols() - 1 && isEmpty(creature.x + 1, creature.y)) {
            cells(creature.x, creature.y) = nullptr;
            creature.x++;
            cells(creature.x, creature.y) = &creature;
            creature.energy--;
            return true;
        }
        return false;
    };

    auto moveLeft = [this](Creature& creature, bool flag) {
        if (flag && creature.x > 0 && isEmpty(creature.x - 1, creature.y)) {
            cells(creature.x, creature.y) = nullptr;
            creature.x--;
            cells(creature.x, creature.y) = &creature;
            creature.energy--;
            return true;
        }
        return false;
    };

    auto photosynthesize = [this](Creature& creature, bool flag) {
        if (flag) {
            creature.energy += std::max(((int) countEmptyAdjacent(creature.x, creature.y)) - 5, 0);
        }
        return false;
        
    };

    auto reproduce = [this](Creature& creature, bool flag) {
        if (flag && creature.energy >= 2 * creature.dna.size()) {
            auto empty_cells = emptyAdjacent(creature.x, creature.y);
            if (!empty_cells.empty()) {
                auto [new_x, new_y] = empty_cells[rand() % empty_cells.size()];
                Creature new_creature(creature.dna, new_x, new_y, creature.dna.size(),
                                      sf::RectangleShape(creature.appearence.getSize()));
                addNew(new_creature);
                creature.energy -= 2 * creature.dna.size();
                return true;
            }
        }
        return false;
    };

    auto nothing = [this](Creature& creature, bool flag) {
        return true;
    };

    static const auto actions = std::array<std::function<bool(Creature&, bool)>, 7>{
        nothing,
        moveUp,
        moveDown,
        moveRight,
        moveLeft,
        photosynthesize,
        reproduce
    };

    bool flag = true;
    for (Action action : creature.dna) {
        flag = actions[static_cast<int>(action)](creature, flag);
    }

    if (creature.energy-- > 0) {
        return true;
    } else {
        return false;
    }
}