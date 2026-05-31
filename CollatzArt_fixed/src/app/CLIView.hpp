#pragma once
#include "Command.hpp"
#include "CommandQueue.hpp"
#include <string>
#include <atomic>

// Snapshot of the current renderer state shown in the CLI menu.
// CLIView keeps a local copy that it updates whenever it sends a command,
// without ever reading from the render thread.
struct DisplayState
{
    bool        playing     { false };
    float       angle       { 15.f };   // degrees
    float       segmentLen  { 8.f };
    Command::SegmentMode segmentMode { Command::SegmentMode::Constant };
    Command::ColorMode   colorMode   { Command::ColorMode::Fixed };
    int         speed       { 4 };      // 1..8
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

private:
    void printMenu() const;
    bool parseLine(const std::string& line);  // true → keep running, false → quit

    CommandQueue<Command>& queue_;
    DisplayState           state_;
    std::atomic<bool>      stop_{ false };
};
