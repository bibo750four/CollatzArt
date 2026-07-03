#include "AppController.hpp"
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/Event.hpp>
#include <iostream>

AppController::AppController(CommandQueue<Command>& queue, CLIView& cliView)
    : commandQueue_(queue)
    , cliView_(cliView)
    , window_(getInitialVideoMode(), kTitle)
{
    window_.setFramerateLimit(kFPS);
}

sf::VideoMode AppController::getInitialVideoMode()
{
    // Use full desktop resolution
    return sf::VideoMode::getDesktopMode();
}

// ─────────────────────────────────────────────────────────────
void AppController::run(const CollatzCollection& collection,
                        const RenderConfig&      initialConfig)
{
    config_ = initialConfig;
    try {
        rebuild(collection);      // initial build
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize renderer: " << e.what() << "\n";
        throw;
    }
    playback_.play();

    sf::Clock clock;
    while (window_.isOpen()) {
        float dt = clock.restart().asSeconds();

        // 1 — SFML events
        while (const auto event = window_.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window_.close();
                cliView_.requestStop();
            }
        }

        // 2 — commands from the CLI thread
        while (auto cmd = commandQueue_.tryPop())
            applyCommand(*cmd, collection);

        // 3 — advance animation
        int steps = playback_.tick(dt);
        if (steps > 0) {
            renderer_.update(steps);
            if (renderer_.isDone())
                playback_.notifyDone();
        }

        // 4 — render
        window_.clear(sf::Color::White);
        renderer_.draw(window_);
        window_.display();
    }
}

// ─────────────────────────────────────────────────────────────
void AppController::applyCommand(const Command& cmd,
                                  const CollatzCollection& collection)
{
    switch (cmd.type) {
        case Command::Type::Play:       playback_.play();              break;
        case Command::Type::Pause:      playback_.pause();             break;

        case Command::Type::Reset:
            playback_.reset();
            renderer_.reset();
            playback_.play();
            break;

        case Command::Type::SpeedUp:
            config_.speed = std::min(8, config_.speed + 1);
            playback_.setSpeed(config_.speed);
            break;

        case Command::Type::SpeedDown:
            config_.speed = std::max(1, config_.speed - 1);
            playback_.setSpeed(config_.speed);
            break;

        case Command::Type::SetAngle:
            config_.angle = cmd.floatVal;
            config_.evenAngle = cmd.floatVal;
            config_.oddAngle = cmd.floatVal;
            try {
                renderer_.applyConfig(config_, window_.getSize());
                playback_.reset();
                playback_.play();
            } catch (const std::exception& e) {
                std::cerr << "Error applying config: " << e.what() << "\n";
            }
            break;

        // Handle setting the even angle
        case Command::Type::SetEvenAngle:
            config_.evenAngle = cmd.floatVal;
            try {
                renderer_.applyConfig(config_, window_.getSize());
                playback_.reset();
                playback_.play();
            } catch (const std::exception& e) {
                std::cerr << "Error applying config: " << e.what() << "\n";
            }
            break;

        // Handle setting the odd angle
        case Command::Type::SetOddAngle:
            config_.oddAngle = cmd.floatVal;
            try {
                renderer_.applyConfig(config_, window_.getSize());
                playback_.reset();
                playback_.play();
            } catch (const std::exception& e) {
                std::cerr << "Error applying config: " << e.what() << "\n";
            }
            break;

        case Command::Type::SetSegmentLen:
            config_.segmentLen = cmd.floatVal;
            try {
                renderer_.applyConfig(config_, window_.getSize());
                playback_.reset();
                playback_.play();
            } catch (const std::exception& e) {
                std::cerr << "Error applying config: " << e.what() << "\n";
            }
            break;

        case Command::Type::SetSegmentMode:
            config_.segmentMode = cmd.segmentMode;
            try {
                renderer_.applyConfig(config_, window_.getSize());
                playback_.reset();
                playback_.play();
            } catch (const std::exception& e) {
                std::cerr << "Error applying config: " << e.what() << "\n";
            }
            break;

        case Command::Type::SetColorMode:
            config_.colorMode = cmd.colorMode;
            try {
                renderer_.recolor(config_);   // recolor only, do not reset animation
            } catch (const std::exception& e) {
                std::cerr << "Error recoloring: " << e.what() << "\n";
            }
            break;

        case Command::Type::SetAnimationMode:
            try {
                renderer_.setAnimationMode(cmd.animationMode, collection);
                playback_.reset();
                playback_.play();
            } catch (const std::exception& e) {
                std::cerr << "Error setting animation mode: " << e.what() << "\n";
            }
            break;

        case Command::Type::ToggleFullscreen:
            try {
                toggleFullscreen(collection);
            } catch (const std::exception& e) {
                std::cerr << "Error toggling fullscreen: " << e.what() << "\n";
            }
            break;

        case Command::Type::Quit:
            window_.close();
            break;
    }
}

// ─────────────────────────────────────────────────────────────
void AppController::toggleFullscreen(const CollatzCollection& collection)
{
    isFullscreen_ = !isFullscreen_;
    if (isFullscreen_) {
        auto modes = sf::VideoMode::getFullscreenModes();
        if (modes.empty()) {
            std::cerr << "No fullscreen modes available. Using windowed mode.\n";
            // Fallback to windowed mode
            window_.create(getInitialVideoMode(), kTitle);
        } else {
            window_.create(modes[0], kTitle, sf::State::Fullscreen);
        }
    } else {
        window_.create(getInitialVideoMode(), kTitle);
    }

    window_.setFramerateLimit(kFPS);
    rebuild(collection);
    playback_.reset();
    playback_.play();
}

void AppController::rebuild(const CollatzCollection& collection)
{
    try {
        renderer_.build(collection, config_, window_.getSize());
    } catch (const std::exception& e) {
        std::cerr << "Error rebuilding renderer: " << e.what() << "\n";
        throw; // Re-throw to be handled by caller
    }
}

