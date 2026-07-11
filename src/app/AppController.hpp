#pragma once
#include "CommandQueue.hpp"
#include "Command.hpp"
#include "CLIView.hpp"
#include "PlaybackController.hpp"
#include "render/FeatherRenderer.hpp"
#include "render/RenderConfig.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <vector>
#include <string>

class AppController {
public:
    AppController(CommandQueue<Command>& queue, CLIView& cliView);
    void run(const CollatzCollection& collection, const RenderConfig& initialConfig, int64_t range = 0, int64_t step = 0);
    /**
     * Runs the application in batch mode using a job file.
     * @param jobFile Path to the job file.
     */
    void runBatchMode(const std::string& jobFile);

private:
    sf::VideoMode getInitialVideoMode();
    /**
     * Parses a job file into a list of RenderJob objects.
     * @param jobFile Path to the job file.
     * @return List of parsed RenderJob objects.
     * @throws std::runtime_error If the file cannot be read or contains invalid jobs.
     */
    std::vector<RenderJob> parseJobFile(const std::string& jobFile);
    /**
     * Applies a given command to the current application state.
     * Handles commands including playback control, toggling fullscreen,
     * rebuilding the renderer, and updating render parameters.
     * New commands handled:
     *   - Command::Type::SetEvenAngle: sets the angle for even numbers.
     *   - Command::Type::SetOddAngle: sets the angle for odd numbers.
     */
    void applyCommand(const Command& cmd, const CollatzCollection& collection);
    void toggleFullscreen(const CollatzCollection& collection);
    void rebuild(const CollatzCollection& collection);

    CommandQueue<Command>& commandQueue_;
    CLIView&               cliView_;
    sf::RenderWindow       window_;
    FeatherRenderer        renderer_;
    PlaybackController     playback_;
    RenderConfig           config_;
    bool                   isFullscreen_{ false };

    static constexpr auto kTitle  = "Collatz Feather";
    static constexpr unsigned kFPS = 60;
};
