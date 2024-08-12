#include <omp.h>

#ifndef ecosystem_supervisor_header
    #define ecosystem_supervisor_header
    #include "ecosystem_supervisor.h"
#endif

#ifndef fps_counter_header
    #define fps_counter_header
    #include "../include/fps_counter.h"
#endif

class EvolutionGameApp {
    friend class EcosystemSupervisor;
    friend class Ecosystem;
private:
    Ecosystem ecosystem;
    Eigen::MatrixX<std::list<Creature>> externalExchangeBuffer;
    
    FpsCounter fps_counter;

public:
    EvolutionGameApp(uint cells_x, uint cells_y, uint cell_size);
    void run(uint seed);
};