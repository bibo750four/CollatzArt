#include "AppController.hpp"
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/Event.hpp>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

AppController::AppController(CommandQueue<Command>& queue, CLIView& cliView)
    : commandQueue_(queue)
    , cliView_(cliView)
    , window_(sf::VideoMode(getInitialVideoMode()), kTitle, sf::State::Windowed, sf::ContextSettings(0, 0, 8))
{
    window_.setFramerateLimit(kFPS);
    // Enable vertical sync to reduce tearing and improve smoothness
    window_.setVerticalSyncEnabled(true);
}

sf::VideoMode AppController::getInitialVideoMode()
{
    // Use full desktop resolution
    return sf::VideoMode::getDesktopMode();
}

// ─────────────────────────────────────────────────────────────
void AppController::run(const CollatzCollection& collection,
                        const RenderConfig&      initialConfig,
                        int64_t range,
                        int64_t step)
{
    config_ = initialConfig;
    cliView_.setRangeStep(range, step);
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
        window_.clear(config_.backgroundColor);
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

        case Command::Type::SetRenderColor:
            config_.fixedColor = cmd.color;
            renderer_.recolor(config_);
            break;

        case Command::Type::SetBackgroundColor:
            config_.backgroundColor = cmd.color;
            break;

        case Command::Type::BatchMode:
            runBatchMode(cmd.stringVal);
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

// ─────────────────────────────────────────────────────────────
std::vector<RenderJob> AppController::parseJobFile(const std::string& jobFile)
{
    std::vector<RenderJob> jobs;
    std::ifstream file(jobFile);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open job file: " + jobFile);
    }

    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        if (line.empty() || line[0] == '#') {
            continue;
        }

        RenderJob job;
        std::istringstream iss(line);
        std::string token;
        bool hasRange = false;
        bool hasStep = false;

        while (std::getline(iss, token, ' ')) {
            size_t delimiterPos = token.find('=');
            if (delimiterPos == std::string::npos) {
                std::cerr << "Warning: Invalid token in job file: '" << token << "'. Skipping.\n";
                continue;
            }

            std::string key = token.substr(0, delimiterPos);
            std::string value = token.substr(delimiterPos + 1);

            try {
                if (key == "range") {
                    job.range = std::stoll(value);
                    hasRange = true;
                } else if (key == "step") {
                    job.step = std::stoll(value);
                    hasStep = true;
                } else if (key == "evenAngle") {
                    job.evenAngle = std::stof(value);
                } else if (key == "oddAngle") {
                    job.oddAngle = std::stof(value);
                } else if (key == "color") {
                    job.color = parseColor(value);
                } else if (key == "background") {
                    job.background = parseColor(value);
                } else if (key == "speed") {
                    job.speed = std::stoi(value);
                    job.speed = std::clamp(job.speed, 1, 8);
                } else if (key == "mode") {
                    if (value == "parallel") {
                        job.mode = Command::AnimationMode::Parallel;
                    } else if (value == "sequential") {
                        job.mode = Command::AnimationMode::Sequential;
                    } else {
                        std::cerr << "Warning: Invalid animation mode '" << value << "'. Using 'parallel'.\n";
                    }
                } else if (key == "renderType") {
                    job.renderType = value;
                }
            } catch (const std::exception& e) {
                std::cerr << "Warning: Failed to parse '" << key << "' in job file. Error: " << e.what() << "\n";
            }
        }

        if (!hasRange || !hasStep) {
            std::cerr << "Warning: Job missing required 'range' or 'step'. Skipping.\n";
            continue;
        }

        jobs.push_back(job);
    }

    if (jobs.empty()) {
        throw std::runtime_error("No valid jobs found in file: " + jobFile);
    }

    return jobs;
}

// ─────────────────────────────────────────────────────────────
void AppController::runBatchMode(const std::string& jobFile)
{
    std::vector<RenderJob> jobs;
    try {
        jobs = parseJobFile(jobFile);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return;
    }

    // Enter fullscreen mode for batch rendering
    if (!isFullscreen_) {
        toggleFullscreen(CollatzCollection{});  // Pass empty collection (will be rebuilt per job)
    }

    bool running = true;
    while (running && window_.isOpen()) {
        for (const auto& job : jobs) {
            if (!window_.isOpen()) {
                running = false;
                break;
            }

            // Skip unsupported render types
            if (job.renderType != "feather") {
                std::cerr << "Warning: Render type '" << job.renderType << "' not supported. Using 'feather'.\n";
            }

            // Generate sequences
            CollatzCollection collection;
            try {
                collection = CollatzEngine::generate(job.range, job.step);
            } catch (const std::exception& e) {
                std::cerr << "Error generating sequences: " << e.what() << "\n";
                continue;
            }

            // Apply job config
            RenderConfig config;
            config.evenAngle = job.evenAngle;
            config.oddAngle = job.oddAngle;
            config.fixedColor = job.color;
            config.backgroundColor = job.background;
            config.speed = job.speed;

            // Rebuild renderer with new config
            try {
                renderer_.build(collection, config, window_.getSize());
                renderer_.setAnimationMode(job.mode, collection);
                playback_.setSpeed(config.speed);
                playback_.play();
            } catch (const std::exception& e) {
                std::cerr << "Error rebuilding renderer: " << e.what() << "\n";
                continue;
            }

            // Render loop for this job
            while (!renderer_.isDone() && window_.isOpen()) {
                while (auto event = window_.pollEvent()) {
                    if (event->is<sf::Event::Closed>() || 
                        (event->is<sf::Event::KeyPressed>() && event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape)) {
                        window_.close();
                        running = false;
                    }
                }

                // Update and draw
                int steps = playback_.tick(1.0f / 60.0f);  // Assume 60 FPS
                if (steps > 0) {
                    renderer_.update(steps);
                }

                window_.clear(config.backgroundColor);
                renderer_.draw(window_);
                window_.display();
            }
        }
    }
}

