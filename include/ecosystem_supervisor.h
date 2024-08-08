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
    uint id;
    uint xOrigin;
    uint yOrigin;
    uint xSize;
    uint ySize;

    //helper structures
    SplitMix64 randGen;
    void kill(Creature& creature);
    Eigen::Vector2<uint> getChunkIdx(uint x, uint y);
    void checkChunkBounds(std::list<Creature>::iterator& it);
    void readDna(std::list<Creature>::iterator& it);

    //gamefield procession
    void processChunk();
    void manageLostOnes();
    void renderChunk();

public:
    EcosystemSupervisor(Ecosystem& ecosystem);
    
    void doTimeStep();
};