#include <iostream>
#include <thread>
#include <climits>
#include "CollatzEngine.hpp"
#include "app/CommandQueue.hpp"
#include "app/CLIView.hpp"
#include "app/AppController.hpp"
#include "render/RenderConfig.hpp"

// Safety limits to prevent memory exhaustion
constexpr int64_t MAX_RANGE = 10000;
constexpr int64_t MAX_STEP = 10000;
constexpr int64_t MAX_START_VALUE = 1000000;


int main(int argc, char* argv[])
{
    // Check for batch mode (--file argument)
    if (argc > 1 && std::string(argv[1]) == "--file" && argc > 2) {
        CommandQueue<Command> queue;
        CLIView cli(queue);
        AppController app(queue, cli);

        // Run batch mode directly (no CLI thread)
        try {
            app.runBatchMode(argv[2]);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << '\n';
            return 1;
        }

        return 0;
    }

    // Interactive mode
    std::cout << "For how many integers do you want to calculate the Collatz sequence?\n";
    std::cout << "(Maximum: " << MAX_RANGE << ")\n";
    int64_t range{};
    if (!(std::cin >> range)) {
        std::cout << "Invalid input for range. Please enter a number.\n";
        return 1;
    }
    if (range < 1) { std::cout << "Range must be at least 1.\n"; return 1; }
    if (range > MAX_RANGE) {
        std::cout << "Range too large. Maximum allowed is " << MAX_RANGE << ".\n";
        return 1;
    }

    std::cout << "What stepping between integers (start 1, next is 1 + step)?\n";
    std::cout << "(Maximum: " << MAX_STEP << ")\n";
    int64_t step{};
    if (!(std::cin >> step)) {
        std::cout << "Invalid input for step. Please enter a number.\n";
        return 1;
    }

    if (step < 1) { std::cout << "Stepping must be at least 1.\n"; return 1; }
    if (step > MAX_STEP) {
        std::cout << "Step too large. Maximum allowed is " << MAX_STEP << ".\n";
        return 1;
    }

    // Check for overflow in maxStart calculation: 1 + (range - 1) * step
    if (range > 1) {
        if (step > (INT64_MAX - 1) / (range - 1)) {
            std::cout << "Range and step combination too large - would cause overflow.\n";
            return 1;
        }
    }

    int64_t maxStart = 1 + (range - 1) * step;
    
    // Additional safety check: limit the maximum starting value
    if (maxStart > MAX_START_VALUE) {
        std::cout << "Maximum starting number " << maxStart << " exceeds safety limit of " 
                  << MAX_START_VALUE << ". Please use smaller range or step.\n";
        return 1;
    }
    
    std::cout << "Generating " << range << " sequences, max starting number: " << maxStart << '\n';



    CollatzCollection collection;
    try {
        collection = CollatzEngine::generate(range, step);
    } catch (const std::overflow_error& e) {
        std::cerr << "Error generating sequences: " << e.what() << "\n";
        std::cerr << "Try using smaller range or step values.\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error generating sequences: " << e.what() << "\n";
        return 1;
    }

    // quick debug: print the stopping time and peak value for each sequence
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

    // Thread-safe: consume the newline before the CLI thread starts
    {
        ConsoleMutex::Lock lock;
        std::cin.ignore();
    }

    std::thread cliThread([&cli]() { cli.run(); });

    try {
        app.run(collection, RenderConfig{}, range, step);   // blocks until the window is closed
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        cli.requestStop();
        cliThread.join();
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred.\n";
        cli.requestStop();
        cliThread.join();
        return 1;
    }

    cli.requestStop();
    cliThread.join();

    return 0;
}
