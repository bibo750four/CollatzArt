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

static std::size_t utf8Len(const std::string& s)
{
    std::size_t count = 0;
    for (unsigned char c : s)
        if ((c & 0xC0u) != 0x80u) ++count;  // conta solo i byte "leading", non i continuation
    return count;
}


// Barra velocità visuale  ████░░░░
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
        case Command::ColorMode::Fixed:       return "fisso";
        case Command::ColorMode::PerSequence: return "per sequenza";
        case Command::ColorMode::PerParity:   return "per parità";
    }
    return "";
}

static std::string segmentModeStr(Command::SegmentMode m)
{
    return m == Command::SegmentMode::Constant ? "costante" : "decrescente";
}

// ─────────────────────────────────────────────
CLIView::CLIView(CommandQueue<Command>& queue)
    : queue_(queue) {}

void CLIView::run()
{
    printMenu();
    std::string line;
    while (!stop_.load() && std::getline(std::cin, line))
    {
        if (!parseLine(line)) break;
        printMenu();
    }
    // se usciamo per 'q', avvisiamo il thread principale
    if (!stop_.load())
        queue_.push({ Command::Type::Quit });
}


void CLIView::printMenu() const
{
    clearScreen();

    const int W = 46;

    // ripete una stringa UTF-8 n volte (non un char — \u2550 è 3 byte)
    auto rep = [](const std::string& s, int n) {
        std::string r;
        r.reserve(s.size() * static_cast<std::size_t>(n));
        for (int i = 0; i < n; ++i) r += s;
        return r;
    };

    auto row = [&](const std::string& s) {
        int padding = (W - 4) - static_cast<int>(utf8Len(s));
        if (padding < 0) padding = 0;   // stringa più lunga del box: tronca il padding
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

    row("Stato:    " + std::string(state_.playing ? "PLAY \u25B6" : "PAUSA \u23F8"));

    {
        std::ostringstream ss;
        ss << "Angolo \u03B8: " << std::fixed << std::setprecision(1)
           << state_.angle << "\u00B0";
        row(ss.str());
    }
    {
        std::ostringstream ss;
        ss << "Segmento: " << std::fixed << std::setprecision(1)
           << state_.segmentLen << "  [" << segmentModeStr(state_.segmentMode) << "]";
        row(ss.str());
    }

    row("Colore:   " + colorModeStr(state_.colorMode));
    row("Velocit\u00E0: " + speedBar(state_.speed)
        + "  (" + std::to_string(state_.speed) + "/8)");

    sep();
    row("COMANDI");
    row("p          \u2192 play / pausa");
    row("r          \u2192 reset");
    row("a <gradi>  \u2192 angolo  (es. a 20.0)");
    row("l <val>    \u2192 lunghezza segmento");
    row("lc / ld    \u2192 costante / decrescente");
    row("cf/cs/cp   \u2192 colore fisso/sequenza/parit\u00E0");
    row("+  / -     \u2192 velocit\u00E0");
    row("f          \u2192 fullscreen");
    row("q          \u2192 esci");

    std::cout << "\u255A" << rep("\u2550", W - 2) << "\u255D\n";
    std::cout << "> " << std::flush;
}


bool CLIView::parseLine(const std::string& line)
{
    std::istringstream ss(line);
    std::string token;
    ss >> token;
    if (token.empty()) return true;

    // --- play / pausa ---
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

    // --- angolo ---
    if (token == "a") {
        float val;
        if (ss >> val && val > 0.f && val < 180.f) {
            state_.angle = val;
            queue_.push({ Command::Type::SetAngle, val });
        } else {
            std::cout << "Valore non valido. Usa: a <gradi>  (0 < θ < 180)\n";
            std::cin.ignore();
        }
        return true;
    }

    // --- lunghezza segmento ---
    if (token == "l") {
        float val;
        if (ss >> val && val > 0.f) {
            state_.segmentLen = val;
            queue_.push({ Command::Type::SetSegmentLen, val });
        } else {
            std::cout << "Valore non valido. Usa: l <numero positivo>\n";
            std::cin.ignore();
        }
        return true;
    }

    // --- modalità segmento ---
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

    // --- modalità colore ---
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

    // --- velocità ---
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

    // --- quit ---
    if (token == "q") return false;

    // comando sconosciuto: stampa avviso senza ridisegnare il menu
    std::cout << "Comando non riconosciuto: '" << token << "'\n";
    std::cout << "Premi Invio per continuare...";
    std::cin.ignore();
    return true;
}
