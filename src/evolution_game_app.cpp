#include <iostream>

#ifndef evolution_game_app_header
    #define evolution_game_app_header
    #include "../include/evolution_game_app.h"
#endif

EvolutionGameApp::EvolutionGameApp(uint cells_x, uint cells_y, uint cell_size): ecosystem(cells_x, cells_y, cell_size) {

}

std::pair<int, int> closestDivisors(uint n) {
    uint sqrt_n = std::sqrt(n);
    for (uint i = sqrt_n; i > 0; --i) {
        if (n % i == 0) {
            return std::make_pair(i, n / i);
        }
    }
    return std::make_pair(1, n); // if n is prime
}

void EvolutionGameApp::run(uint seed) {
    //#pragma omp parallel
    {
        uint threads_num = omp_get_num_threads();
        uint thread_id = omp_get_thread_num();
        auto [thread_rows, thread_cols] = closestDivisors(threads_num);

        auto supervisor = EcosystemSupervisor(ecosystem, thread_rows, thread_cols, seed);

        if (thread_id == 0) {
            std::vector<Action> dna = {Action::reproduce, Action::nothing, Action::photosynthesize};
            uint32_t color = UINT32_MAX;
            Creature first_life = Creature(dna, color, 1, 0, 0, 1, Eigen::Vector2<uint>(0, 3));
            ecosystem.cells(0, 0) = &first_life;
        }

        while (ecosystem.window.isOpen()) {
            for (uint chunk_idx_1 = 0; chunk_idx_1 < 4; ++chunk_idx_1) {
                supervisor.processChunk(chunk_idx_1);
                #pragma omp barrier
            }
            supervisor.manageLostOnes();
            supervisor.renderChunks();

            if (thread_id == 0) {
                for (auto event = sf::Event{}; ecosystem.window.pollEvent(event);) {
                    if (event.type == sf::Event::Closed)
                        ecosystem.window.close();
                }
                ecosystem.renderWindow();
            }
        }
    }
}