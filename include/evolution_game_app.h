#include <omp.h>

#ifndef ecosystem_supervisor_header
    #define ecosystem_supervisor_header
    #include "ecosystem_supervisor.h"
#endif

class EvolutionGameApp {
    friend class EcosystemSupervisor;
    friend class Ecosystem;
private:
    Ecosystem ecosystem;

public:
    EvolutionGameApp(uint cells_x, uint cells_y, uint cell_size);
    void run(uint seed);
};