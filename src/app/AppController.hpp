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