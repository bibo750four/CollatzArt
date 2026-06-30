#pragma once
#include "CommandQueue.hpp"
#include "Command.hpp"
#include "CLIView.hpp"
#include "PlaybackController.hpp"
#include "render/FeatherRenderer.hpp"
#include <SFML/Graphics/RenderWindow.hpp>

class AppController {
public:
    AppController(CommandQueue<Command>& queue, CLIView& cliView);
    void run(const CollatzCollection& collection, const RenderConfig& initialConfig);

private:
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
