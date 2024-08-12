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
    uint threadsX;
    uint threadsY;

    uint xOrigin;
    uint yOrigin;
    uint xSize;
    uint ySize;

    //helper structures
    SplitMix64 randGen;

    //chunk transition mechanism
    std::vector<std::list<Creature>> internalExchangeBuffer;
    inline std::pair<uint, uint> getChunkId(uint x, uint y);

    inline void moveToInternalBuffer(std::list<Creature>::iterator& it, uint to_id_1, uint from_id_1);
    inline void moveToExternalBuffer(std::list<Creature>::iterator& it, uint to_id_0, uint from_id_1);

    inline void emptyInternalBuffer(uint chunk_id_1);
    inline void emptyExternalBuffer(uint chunk_id_0);

    inline void checkAndHandleChunkTransition(std::list<Creature>::iterator& it, uint from_id_1);

    //helper functions
    inline void kill(Creature& it);
    inline std::pair<std::vector<Action>, uint32_t> mutate(const std::vector<Action>& dna, uint32_t color);
    inline void placeNewborn(const std::vector<Action>& parent_dna, const uint32_t parent_color, int energy,
        uint x, uint y, uint parent_chunk_id_1);
    inline void doAction(std::list<Creature>::iterator& it, uint chunk_id_1);

protected:
    //gamefield procession
    void processChunk(uint chunk_id_1);
    void processExchangeBuffers();
    void renderChunks();

    //starting creature
    void addFirstLife(Creature& creature);

public:
    EcosystemSupervisor(Ecosystem& ecosystem, Eigen::MatrixX<std::list<Creature>>& externalExchangeBuffer, uint threads_x, uint threads_y, uint seed);
};