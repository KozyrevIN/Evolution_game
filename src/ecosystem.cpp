#define ecosystem_header
#include "../include/ecosystem.h"

bool Ecosystem::isEmpty(uint x, uint y) {
    return (cells(x, y) == nullptr);
}

uint Ecosystem::countEmptyAdjacent(uint x, uint y) {
    uint count = 0;
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            int newX = x + dx;
            int newY = y + dy;
            if (newX >= 0 && newX < cells.rows() && newY >= 0 && newY < cells.cols()) {
                if (isEmpty(newX, newY)) {
                    count++;
                }
            }
        }
    }
    return count;
}

std::vector<std::pair<uint, uint>> Ecosystem::emptyAdjacent(uint x, uint y) {
    std::vector<std::pair<uint, uint>> emptyCells;
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            int newX = x + dx;
            int newY = y + dy;
            if (newX >= 0 && newX < cells.rows() && newY >= 0 && newY < cells.cols()) {
                if (isEmpty(newX, newY)) {
                    emptyCells.push_back(std::pair<uint, uint>(newX, newY));
                }
            }
        }
    }
    return emptyCells;
}

void Ecosystem::renderCreature(Creature& creature) {
    textureCP(creature.x, creature.y) = creature.color;
}

void Ecosystem::renderWindow() {
    textureGP.update(reinterpret_cast<uint8_t*>(textureCP.data()));
    sf::Sprite sprite(textureGP);
    sprite.setScale(window.getSize().x / textureGP.getSize().x, window.getSize().y / textureGP.getSize().y);
    sprite.setPosition(0, 0);
    window.display();
}

Ecosystem::Ecosystem(uint cells_x, uint cells_y, uint cell_size) {
    cells = Eigen::MatrixX<Creature*>::Constant(cells_x, cells_y, nullptr);
    window.create(sf::VideoMode(cells_x * cell_size, cells_y * cell_size), "Evolution_Game");
    //window.setFramerateLimit(60);
    textureCP = Eigen::MatrixX<uint32_t>::Constant(0);
}

Ecosystem::~Ecosystem() {
    window.close();
}