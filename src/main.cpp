#include <iostream>

#include "../include/life.h"

std::pair<uint, uint> closest_divisors(int n) {
    int sqrt_n = std::sqrt(n);
    for (int i = sqrt_n; i > 0; --i) {
        if (n % i == 0) {
            return std::make_pair(n / i, i);
        }
    }
    return std::make_pair(1, n); // if n is prime
}

int main()
{
    Ecosystem system(1920, 1080, 1);
    system.process();

    return 0;
}

void Ecosystem::process() {
    //thread specific data
    uint x_dim;
    uint y_dim;
    std::list<Creature> creatures;
    //#pragma omp threadprivate(creatures)
    uint x_origin;
    //#pragma omp threadprivate(x_origin)
    uint y_origin;
    //#pragma omp threadprivate(y_origin)

    uint frames = 0;
    auto t1 = std::chrono::high_resolution_clock::now();
    //#pragma omp parallel
    while (window.isOpen())
    {   
        ++frames;
        for (auto iter = creatures.begin(); iter != creatures.end(); ) {
            if (readDna(*iter)) {
                iter++;
            } else {
                auto kill_iter = iter++;
                kill(kill_iter);
            }
        }

        window.clear();

        for (auto iter = creatures.begin(); iter != creatures.end(); ++iter) {
            display(*iter);
        }

        //#pragma omp 
        window.display();

        for (auto event = sf::Event{}; window.pollEvent(event);)
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        /*
        if (frames >= 100) {
            auto t2 = std::chrono::high_resolution_clock::now();
            auto fps = frames / (std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() / 1000.f);
            frames = 0;
            if (fps < 30) {
                window.close();
                std::cout << fps << " " << creatures.size() << "\n";
            }
            t1 = std::chrono::high_resolution_clock::now();
        }*/
    }

    window.close();
}
