#include <iostream>

#ifndef evolution_game_app_header
    #define evolution_game_app_header
    #include "../include/evolution_game_app.h"
#endif

EvolutionGameApp::EvolutionGameApp(uint cells_x, uint cells_y, uint cell_size): ecosystem(cells_x, cells_y, cell_size) {
    externalExchangeBuffer = Eigen::MatrixX<std::list<Creature>>();
}

std::pair<int, int> closestDivisors(uint n) {
    uint sqrt_n = std::sqrt(n);
    for (uint i = sqrt_n; i > 0; --i) {
        if (n % i == 0) {
            return std::make_pair(n / i, i);
        }
    }
    return std::make_pair(1, n); // if n is prime
}

void EvolutionGameApp::run(uint seed) {
    #pragma omp parallel
    {
        uint threads_num = omp_get_num_threads();
        uint thread_id = omp_get_thread_num();
        auto [thread_rows, thread_cols] = closestDivisors(threads_num);

        if (thread_id == 0) {
            externalExchangeBuffer = Eigen::MatrixX<std::list<Creature>>::Constant(threads_num, threads_num, std::list<Creature>());
        }

        auto supervisor = EcosystemSupervisor(ecosystem, externalExchangeBuffer, thread_rows, thread_cols, seed);

        {
            std::vector<Action> dna = {Action::reproduce, Action::nothing, Action::photosynthesize};
            uint32_t color = UINT32_MAX;
            uint x = ecosystem.cells.rows() / 2;
            uint y = ecosystem.cells.cols() / 2;
            int energy = 1;
            uint8_t direction = 1;
            auto chunk_id = supervisor.getChunkId(x, y);

            Creature first_life = Creature(dna, color, energy, direction, x, y, chunk_id);
            supervisor.addFirstLife(first_life);
        }

        while (ecosystem.window.isOpen()) {
            if (thread_id == 0) {
                ecosystem.renderWindow();

                for (auto event = sf::Event{}; ecosystem.window.pollEvent(event);) {
                    if (event.type == sf::Event::Closed)
                        ecosystem.window.close();
                }
            }

            for (uint chunk_id_1 = 0; chunk_id_1 < 3; ++chunk_id_1) {
                supervisor.processChunk(chunk_id_1);
                #pragma omp barrier
            }
            supervisor.processChunk(3);

            supervisor.processExchangeBuffers();
            supervisor.renderChunks();
        }
    }
}