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
    friend class EvolutionGameApp;
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
    void checkLost(uint i, uint j, uint chunk_idx_1);

protected:
    //gamefield procession
    void processChunk(uint chunk_idx_1);
    void manageLostOnes();
    void renderChunks();

public:
    EcosystemSupervisor(Ecosystem& ecosystem, uint thread_rows, uint thread_cols, uint seed);
};