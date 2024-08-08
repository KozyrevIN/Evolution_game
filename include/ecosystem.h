#include <SFML/Graphics.hpp>
#include <eigen3/Eigen/Dense>

#ifndef creature_header
    #define creature_header
    #include "creature.h"
#endif

class Ecosystem {
    friend class EcosystemSupervisor;
    friend class EvolutionGameApp;
protected:
    Eigen::MatrixX<Creature*> cells;
    Eigen::MatrixX<uint32_t> textureCP;
    sf::Texture textureGP;
    sf::RenderWindow window;

    //helper functions
    bool isEmpty(uint x, uint y);
    uint countEmptyAdjacent(uint x, uint y);
    std::vector<std::pair<uint, uint>> emptyAdjacent(uint x, uint y);

    //rendering
    void renderCreature(Creature& creature);
    void renderWindow();

public:
    Ecosystem(uint cells_x, uint cells_y, uint cell_size);
    ~Ecosystem();
};