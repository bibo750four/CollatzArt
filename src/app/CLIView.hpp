#pragma once
#include "Command.hpp"
#include "CommandQueue.hpp"
#include <SFML/Graphics/Color.hpp>
#include <string>
#include <atomic>
#include <mutex>

// Snapshot of the current renderer state shown in the CLI menu.
// CLIView keeps a local copy that it updates whenever it sends a command,
// without ever reading from the render thread.
struct DisplayState
{
    bool        playing     { false };
    float       angle       { 5.f };   // degrees
    float       evenAngle   { 12.f };   // degrees
    float       oddAngle    { 24.f };   // degrees
    float       branchAngle { 30.f };  // degrees (tree-specific)
    float       segmentLen  { 1.f };
    Command::SegmentMode segmentMode { Command::SegmentMode::Constant };
    Command::ColorMode   colorMode   { Command::ColorMode::Fixed };
    Command::AnimationMode animationMode { Command::AnimationMode::Parallel };
    int         speed       { 4 };      // 1..8
    std::string rendererType { "feather" }; // Active renderer type
};

// Global mutex for thread-safe console I/O
// Used by both CLI thread and main thread to prevent race conditions on std::cin/std::cout
class ConsoleMutex {
public:
    static std::mutex& get() {
        static std::mutex instance;
        return instance;
    }
    
    // RAII lock for console I/O
    class Lock {
    public:
        Lock() : mutex_(get()) { mutex_.lock(); }
        ~Lock() { mutex_.unlock(); }
        Lock(const Lock&) = delete;
        Lock& operator=(const Lock&) = delete;
    private:
        std::mutex& mutex_;
    };
};

class CLIView
{
public:
    explicit CLIView(CommandQueue<Command>& queue);

    // Starts the input loop — must be called on a secondary thread.
    // Blocks until the user types 'q'.
    void run();

    // Lets the main thread signal that the CLI should stop
    // (e.g. the user closed the SFML window).
    void requestStop() { stop_.store(true); }
    void setRangeStep(int64_t range, int64_t step) { range_ = range; step_ = step; }

private:
    void printMenu() const;
    bool parseLine(const std::string& line);  // true → keep running, false → quit
    void handleRenderColorCommand();
    void handleBackgroundColorCommand();
    void handleBatchCommand(const std::vector<std::string>& tokens);
    void handleStoreCommand(const std::vector<std::string>& tokens);
    void handleRendererCommand();
    sf::Color selectColor();
    std::string currentSettingsToJobString() const;

    CommandQueue<Command>& queue_;
    DisplayState           state_;
    std::atomic<bool>      stop_{ false };
    int64_t                range_{ 0 };
    int64_t                step_{ 0 };
    sf::Color              currentColor_{ sf::Color::Black };
    sf::Color              currentBackground_{ sf::Color::White };
};

