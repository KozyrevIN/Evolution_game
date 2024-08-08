#include <list>

#ifndef ecosystem_header
    #define ecosystem_header
    #include "ecosystem.h"
#endif

#ifndef split_mix_64_header
    #define split_mix_64_header
    #include "split_mix_64.h"
#endif

class EcosystemSupervisor {
    friend class Ecosystem;
private:
    Ecosystem& ecosystem;
    std::vector<std::list<Creature>> creatures;

    //supervisor's control region
    uint xOrigin;
    uint yOrigin;
    uint xSize;
    uint ySize;

    //helper functions
    splitMix64 randGen;
    Eigen::Vector2<uint> getChunk(uint x, uint y);
    void kill(Creature& creature);
    void addNew(Creature& creature);
    void readDna(Creature& creature);

    //gamefield procession
    void processChunk();
    void manageLostOnes();
    void renderChunk();

public:
    EcosystemSupervisor(Ecosystem& ecosystem);
    
    void doTimeStep();
};