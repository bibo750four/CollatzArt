#pragma once
#include "RenderConfig.hpp"
#include "engine/CollatzEngine.hpp"   // per CollatzCollection
#include <SFML/Graphics/RenderWindow.hpp>

class Renderer {
public:
    virtual ~Renderer() = default;

    // Costruisce la struttura dati interna dalla collection
    virtual void build(const CollatzCollection&,
                       const RenderConfig&,
                       sf::Vector2u windowSize) = 0;

    // Ricalcola solo la geometria (angolo/lunghezza cambiati), azzeraanimazione
    virtual void applyConfig(const RenderConfig&, sf::Vector2u windowSize) = 0;

    // Ricolora le forme già disegnate senza toccare la geometria
    virtual void recolor(const RenderConfig&) = 0;

    // Avanza l'animazione di N passi
    virtual void update(int steps) = 0;

    virtual void draw(sf::RenderWindow&) const = 0;
    virtual void reset() = 0;
    virtual bool isDone() const = 0;
};
