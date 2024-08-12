#include <iostream>
#include <format>

#ifndef evolution_game_app_header
    #define evolution_game_app_header
    #include "../include/evolution_game_app.h"
#endif

EvolutionGameApp::EvolutionGameApp(uint cells_x, uint cells_y, uint cell_size): ecosystem(cells_x, cells_y, cell_size) {
    externalExchangeBuffer = Eigen::MatrixX<std::list<Creature>>();
    fps_counter = FpsCounter();
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
        auto [threads_x, threads_y] = closestDivisors(threads_num);
        if (ecosystem.cells.rows() < ecosystem.cells.cols()) {
            std::swap(threads_x, threads_y);
        }

        if (thread_id == 0) {
            externalExchangeBuffer = Eigen::MatrixX<std::list<Creature>>::Constant(threads_num, threads_num, std::list<Creature>());
        }

        auto supervisor = EcosystemSupervisor(ecosystem, externalExchangeBuffer, threads_x, threads_y, seed);

        {
            std::vector<Action> dna = {Action::reproduce, Action::nothing, Action::photosynthesize};
            uint32_t color = 0xFF888888;
            uint x = ecosystem.cells.rows() / 2;
            uint y = ecosystem.cells.cols() / 2;
            int energy = 10;
            uint8_t direction = 1;

            Creature first_life = Creature(dna, color, energy, direction, x, y);
            supervisor.addFirstLife(first_life);
        }

        while (ecosystem.window.isOpen()) {

            #pragma omp barrier
            if (thread_id == 0) {
                ecosystem.renderWindow(std::format("{:.1f}", fps_counter.getFps()));

                for (auto event = sf::Event{}; ecosystem.window.pollEvent(event);) {
                    if (event.type == sf::Event::Closed)
                        ecosystem.window.close();
                }
            }

            for (uint chunk_id_1 = 0; chunk_id_1 < 4; ++chunk_id_1) {
                supervisor.processChunk(chunk_id_1);
                #pragma omp barrier
            }

            supervisor.processExchangeBuffers();
            supervisor.renderChunks();
        }
    }
}