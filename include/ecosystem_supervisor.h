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
    Eigen::MatrixX<std::list<Creature>>& externalExchangeBuffer;
    std::vector<std::list<Creature>> creatures;

    //supervisor's control region
    uint id;
    uint threadRows;
    uint threadCols;

    uint xOrigin;
    uint yOrigin;
    uint xSize;
    uint ySize;

    //helper structures
    SplitMix64 randGen;

    //chunk transition mechanism
    std::vector<std::list<Creature>> internalExchangeBuffer;
    std::pair<uint, uint> getChunkId(uint x, uint y);
    void putToInternalBuffer(std::list<Creature>::iterator& it, uint chunk_id_1);
    void putToExternalBuffer(std::list<Creature>::iterator& it, std::pair<uint, uint> chunk_id);
    void checkChunkBounds(std::list<Creature>::iterator& it);

    //helper functions
    void kill(Creature& it);
    void placeNewborn(const std::vector<Action>& parent_dna, const uint32_t parent_color,
        std::pair<uint, uint> parent_chunk_id, uint x, uint y, int energy);
    void doAction(std::list<Creature>::iterator& it);

protected:
    //gamefield procession
    void processChunk(uint chunk_id_1);
    void processExchangeBuffers();
    void renderChunks();

    //starting creature
    void addFirstLife(const Creature& creature);

public:
    EcosystemSupervisor(Ecosystem& ecosystem, Eigen::MatrixX<std::list<Creature>>& externalExchangeBuffer, uint thread_rows, uint thread_cols, uint seed);
};