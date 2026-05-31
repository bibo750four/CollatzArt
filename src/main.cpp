#include <iostream>
#include <thread>
#include "CollatzEngine.hpp"
#include "app/CommandQueue.hpp"
#include "app/CLIView.hpp"
#include "app/AppController.hpp"
#include "render/RenderConfig.hpp"


int main()
{
    std::cout << "For how many integers do you want to calculate the Collatz sequence?\n";
    int64_t range{};
    std::cin >> range;
    if (range < 1) { std::cout << "Range must be at least 1.\n"; return 1; }

    std::cout << "What stepping between integers (start 1, next is 1 + step)?\n";
    int64_t step{};
    std::cin >> step;
    
    
    if (step < 1) { std::cout << "Stepping must be at least 1.\n"; return 1; }

    
    
    int64_t maxStart = 1 + (range - 1) * step;
    std::cout << "Generating " << range << " sequences, max starting number: " << maxStart << '\n';

    auto collection = CollatzEngine::generate(range, step);

    // quick debug: print the total sequence steps and peak for each sequence
    for (const auto& [start, seq] : collection)
    {
        std::cout << "n=" << start
                  << "  steps=" << CollatzEngine::stoppingTime(seq)
                  << "  peak="  << CollatzEngine::maxValue(seq)
                  << '\n';
    }

    CommandQueue<Command> queue;
    CLIView               cli(queue);
    AppController         app(queue, cli);

    std::cin.ignore(); // consumes newline before the CLI thread

    
    std::thread cliThread([&cli]() { cli.run(); });

    app.run(collection, RenderConfig{});   // blocks until the window is closed
    cli.requestStop();
    cliThread.join();
    
    return 0;
}
