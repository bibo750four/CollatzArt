#pragma once
#include "Command.hpp"
#include "CommandQueue.hpp"
#include <string>
#include <atomic>

// Stato corrente visualizzato nel menu.
// CLIView ne mantiene una copia locale: si aggiorna ogni volta
// che invia un comando, senza mai leggere dal thread di render.
struct DisplayState
{
    bool        playing     { false };
    float       angle       { 15.f };   // gradi
    float       segmentLen  { 8.f };
    Command::SegmentMode segmentMode { Command::SegmentMode::Constant };
    Command::ColorMode   colorMode   { Command::ColorMode::Fixed };
    int         speed       { 4 };      // 1..8
};

class CLIView
{
public:
    explicit CLIView(CommandQueue<Command>& queue);

    // Avvia il loop — da chiamare su un thread secondario.
    // Blocca finché l'utente non invia 'q'.
    void run();

    // Il thread principale può segnalare di uscire
    // (es. l'utente chiude la finestra SFML).
    void requestStop() { stop_.store(true); }

private:
    void printMenu() const;
    bool parseLine(const std::string& line);  // true → continua, false → quit

    CommandQueue<Command>& queue_;
    DisplayState           state_;
    std::atomic<bool>      stop_{ false };
};