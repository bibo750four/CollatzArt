#include "CLIView.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <unistd.h>

static void clearScreen() {
    if (isatty(STDOUT_FILENO))
        std::cout << "\033[2J\033[H";
    else
        std::cout << "\n---\n";
}

// Returns the number of Unicode codepoints in a UTF-8 string
// (counts only leading bytes, not continuation bytes)
static std::size_t utf8Len(const std::string& s)
{
    std::size_t count = 0;
    for (unsigned char c : s)
        if ((c & 0xC0u) != 0x80u) ++count;
    return count;
}

// Visual speed bar  ████░░░░
static std::string speedBar(int speed)
{
    std::string bar;
    for (int i = 1; i <= 8; ++i)
        bar += (i <= speed) ? "\u2588" : "\u2591";
    return bar;
}

static std::string colorModeStr(Command::ColorMode m)
{
    switch (m) {
        case Command::ColorMode::Fixed:       return "fixed";
        case Command::ColorMode::PerSequence: return "per sequence";
        case Command::ColorMode::PerParity:   return "per parity";
    }
    return "";
}

static std::string segmentModeStr(Command::SegmentMode m)
{
    return m == Command::SegmentMode::Constant ? "constant" : "decreasing";
}

static std::string animationModeStr(Command::AnimationMode m)
{
    switch (m) {
        case Command::AnimationMode::Parallel:    return "parallel";
        case Command::AnimationMode::Sequential:  return "sequential";
    }
    return "";
}

// ─────────────────────────────────────────────
CLIView::CLIView(CommandQueue<Command>& queue)
    : queue_(queue) {}

void CLIView::run()
{
    printMenu();
    std::string line;
    // Maximum input line length to prevent memory exhaustion
    constexpr std::size_t MAX_INPUT_LENGTH = 256;
    
    while (!stop_.load()) {
        // Lock console I/O for reading input
        {
            ConsoleMutex::Lock lock;
            if (!std::getline(std::cin, line)) {
                // EOF or error
                break;
            }
            
            // Truncate if line is too long
            if (line.size() > MAX_INPUT_LENGTH) {
                line = line.substr(0, MAX_INPUT_LENGTH);
            }
        }
        
        if (!parseLine(line)) break;
        printMenu();
    }
    // If we exit because the user typed 'q', notify the main thread
    if (!stop_.load())
        queue_.push({ Command::Type::Quit });
}


void CLIView::printMenu() const
{
    ConsoleMutex::Lock lock; // Thread-safe console output
    clearScreen();

    const int W = 46;

    // Repeats a UTF-8 string n times (not a single char — \u2550 is 3 bytes)
    auto rep = [](const std::string& s, int n) {
        std::string r;
        r.reserve(s.size() * static_cast<std::size_t>(n));
        for (int i = 0; i < n; ++i) r += s;
        return r;
    };

    auto row = [&](const std::string& s) {
        int padding = (W - 4) - static_cast<int>(utf8Len(s));
        if (padding < 0) padding = 0;   // string wider than box: clamp padding
        std::cout << "\u2551  " << s
                  << std::string(static_cast<std::size_t>(padding), ' ')
                  << "\u2551\n";
    };
    
    auto sep = [&]() {
        std::cout << "\u2560" << rep("\u2550", W - 2) << "\u2563\n";
    };

    std::cout << "\u2554" << rep("\u2550", W - 2) << "\u2557\n";
    row("Collatz Feather Renderer");
    sep();

    row("State:  " + std::string(state_.playing ? "PLAY \u25B6" : "PAUSE \u23F8"));

    {
        std::ostringstream ss;
        ss << "Angle \u03B8 (both): " << std::fixed << std::setprecision(1)
           << state_.angle << "\u00B0";
        row(ss.str());
    }
    {
        std::ostringstream ss;
        ss << "Even angle: " << std::fixed << std::setprecision(1)
           << state_.evenAngle << "\u00B0";
        row(ss.str());
    }
    {
        std::ostringstream ss;
        ss << "Odd angle:  " << std::fixed << std::setprecision(1)
           << state_.oddAngle << "\u00B0";
        row(ss.str());
    }
    {
        std::ostringstream ss;
        ss << "Segment: " << std::fixed << std::setprecision(1)
           << state_.segmentLen << "  [" << segmentModeStr(state_.segmentMode) << "]";
        row(ss.str());
    }

    row("Animation: " + animationModeStr(state_.animationMode));
    row("Color:  " + colorModeStr(state_.colorMode));
    row("Speed:  " + speedBar(state_.speed)
        + "  (" + std::to_string(state_.speed) + "/8)");

    sep();
    row("COMMANDS");
    row("p          \u2192 play / pause");
    row("r          \u2192 reset");
    row("a <deg>    \u2192 angle (both even/odd)");
    row("ae <deg>   \u2192 even angle");
    row("ao <deg>   \u2192 odd angle");
    row("l <val>    \u2192 segment length");
    row("lc / ld    \u2192 constant / decreasing");
    row("cf/cs/cp   \u2192 color fixed/sequence/parity");
    row("+  / -     \u2192 speed");
    row("f          \u2192 fullscreen");
    row("cr         → change render color");
    row("batch <file> → run batch jobs from file");
    row("cb         → change background color");
    row("m p / m s   \u2192 animation parallel/sequential");
    row("q          \u2192 quit");

    std::cout << "\u255A" << rep("\u2550", W - 2) << "\u255D\n";
    std::cout << "> " << std::flush;
}

// ─────────────────────────────────────────────────────────────
void CLIView::handleRenderColorCommand()
{
    sf::Color color = selectColor();
    queue_.push({ Command::Type::SetRenderColor, 0.f, Command::ColorMode::Fixed, Command::SegmentMode::Constant, Command::AnimationMode::Parallel, color });
}

void CLIView::handleBackgroundColorCommand()
{
    sf::Color color = selectColor();
    queue_.push({ Command::Type::SetBackgroundColor, 0.f, Command::ColorMode::Fixed, Command::SegmentMode::Constant, Command::AnimationMode::Parallel, color });
}

sf::Color CLIView::selectColor()
{
    ConsoleMutex::Lock lock;
    std::cout << "\nSelect a color:\n";
    std::cout << "  1. Black\n";
    std::cout << "  2. White\n";
    std::cout << "  3. Red\n";
    std::cout << "  4. Green\n";
    std::cout << "  5. Blue\n";
    std::cout << "  6. Yellow\n";
    std::cout << "  7. Magenta\n";
    std::cout << "  8. Cyan\n";
    std::cout << "  9. Custom RGB\n";
    std::cout << "Enter choice (1-9): ";

    int choice;
    std::cin >> choice;
    std::cin.ignore(); // Clear newline

    switch (choice) {
        case 1:  return sf::Color::Black;
        case 2:  return sf::Color::White;
        case 3:  return sf::Color::Red;
        case 4:  return sf::Color::Green;
        case 5:  return sf::Color::Blue;
        case 6:  return sf::Color::Yellow;
        case 7:  return sf::Color::Magenta;
        case 8:  return sf::Color::Cyan;
        case 9: {
            int r, g, b;
            std::cout << "Enter RGB values (0-255) separated by spaces (e.g., '255 100 50'): ";
            std::cin >> r >> g >> b;
            std::cin.ignore(); // Clear newline
            return sf::Color(static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b));
        }
        default:
            std::cout << "Invalid choice. Using Black.\n";
            return sf::Color::Black;
    }
}

// ─────────────────────────────────────────────────────────────
void CLIView::handleBatchCommand(const std::vector<std::string>& tokens)
{
    if (tokens.empty()) {
        std::cout << "Usage: batch <job_file>\n";
        std::cout << "Press Enter to continue...";
        std::cin.ignore();
        return;
    }

    std::string jobFile = tokens[0];
    queue_.push({ Command::Type::BatchMode, 0.f, Command::ColorMode::Fixed, Command::SegmentMode::Constant, Command::AnimationMode::Parallel, sf::Color::Black, jobFile });
}


bool CLIView::parseLine(const std::string& line)
{
    std::istringstream ss(line);
    std::string token;
    ss >> token;
    if (token.empty()) return true;

    // --- play / pause ---
    if (token == "p") {
        state_.playing = !state_.playing;
        queue_.push({ state_.playing ? Command::Type::Play : Command::Type::Pause });
        return true;
    }

    // --- reset ---
    if (token == "r") {
        state_.playing = false;
        queue_.push({ Command::Type::Reset });
        return true;
    }

    // --- angle (both) ---
    if (token == "a") {
        float val;
        if (ss >> val && val > 0.f && val < 180.f) {
            state_.angle = val;
            state_.evenAngle = val;
            state_.oddAngle = val;
            queue_.push({ Command::Type::SetAngle, val });
            queue_.push({ Command::Type::SetEvenAngle, val });
            queue_.push({ Command::Type::SetOddAngle, val });
        } else {
            std::cout << "Invalid value. Use: a <degrees>  (0 < θ < 180)\n";
            std::cin.ignore();
        }
        return true;
    }

    // --- even angle ---
    if (token == "ae") {
        float val;
        if (ss >> val && val > 0.f && val < 180.f) {
            state_.evenAngle = val;
            queue_.push({ Command::Type::SetEvenAngle, val });
        } else {
            std::cout << "Invalid value. Use: ae <degrees>  (0 < θ < 180)\n";
            std::cin.ignore();
        }
        return true;
    }

    // --- odd angle ---
    if (token == "ao") {
        float val;
        if (ss >> val && val > 0.f && val < 180.f) {
            state_.oddAngle = val;
            queue_.push({ Command::Type::SetOddAngle, val });
        } else {
            std::cout << "Invalid value. Use: ao <degrees>  (0 < θ < 180)\n";
            std::cin.ignore();
        }
        return true;
    }

    // --- segment length ---
    if (token == "l") {
        float val;
        if (ss >> val && val > 0.f) {
            state_.segmentLen = val;
            queue_.push({ Command::Type::SetSegmentLen, val });
        } else {
            std::cout << "Invalid value. Use: l <positive number>\n";
            std::cin.ignore();
        }
        return true;
    }

    // --- segment mode ---
    if (token == "lc") {
        state_.segmentMode = Command::SegmentMode::Constant;
        queue_.push({ Command::Type::SetSegmentMode, 0.f,
                      Command::ColorMode::Fixed, Command::SegmentMode::Constant });
        return true;
    }
    if (token == "ld") {
        state_.segmentMode = Command::SegmentMode::Decreasing;
        queue_.push({ Command::Type::SetSegmentMode, 0.f,
                      Command::ColorMode::Fixed, Command::SegmentMode::Decreasing });
        return true;
    }

    // --- animation mode ---
    if (token == "m" && ss >> token) {
        if (token == "p") {
            state_.animationMode = Command::AnimationMode::Parallel;
            queue_.push({ Command::Type::SetAnimationMode, 0.f, Command::ColorMode::Fixed, Command::SegmentMode::Constant, Command::AnimationMode::Parallel });
            return true;
        }
        if (token == "s") {
            state_.animationMode = Command::AnimationMode::Sequential;
            queue_.push({ Command::Type::SetAnimationMode, 0.f, Command::ColorMode::Fixed, Command::SegmentMode::Constant, Command::AnimationMode::Sequential });
            return true;
        }
    }

    // --- color mode ---
    if (token == "cf") {
        state_.colorMode = Command::ColorMode::Fixed;
        queue_.push({ Command::Type::SetColorMode, 0.f, Command::ColorMode::Fixed });
        return true;
    }
    if (token == "cs") {
        state_.colorMode = Command::ColorMode::PerSequence;
        queue_.push({ Command::Type::SetColorMode, 0.f, Command::ColorMode::PerSequence });
        return true;
    }
    if (token == "cp") {
        state_.colorMode = Command::ColorMode::PerParity;
        queue_.push({ Command::Type::SetColorMode, 0.f, Command::ColorMode::PerParity });
        return true;
    }

    // --- render color ---
    if (token == "cr") {
        handleRenderColorCommand();
        return true;
    }

    // --- background color ---
    if (token == "cb") {
        handleBackgroundColorCommand();
        return true;
    }

    // --- speed ---
    if (token == "+") {
        state_.speed = std::min(8, state_.speed + 1);
        queue_.push({ Command::Type::SpeedUp });
        return true;
    }
    if (token == "-") {
        state_.speed = std::max(1, state_.speed - 1);
        queue_.push({ Command::Type::SpeedDown });
        return true;
    }

    // --- fullscreen ---
    if (token == "f") {
        queue_.push({ Command::Type::ToggleFullscreen });
        return true;
    }

    // --- batch ---
    if (token == "batch" && ss >> token) {
        handleBatchCommand({token});
        return true;
    }

    // --- quit ---
    if (token == "q") return false;

    // Unknown command: print a warning without redrawing the menu
    std::cout << "Unknown command: '" << token << "'\n";
    std::cout << "Press Enter to continue...";
    std::cin.ignore();
    return true;
}
