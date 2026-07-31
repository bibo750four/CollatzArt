#pragma once
#include "Renderer.hpp"
#include "FeatherRenderer.hpp"
#include "RenderConfig.hpp"
#include <memory>

/**
 * Factory to create renderers based on the rendererType in RenderConfig.
 */
class RendererFactory {
public:
    /**
     * Creates a renderer based on the rendererType in the config.
     * @param config The render configuration.
     * @return Unique pointer to the created renderer.
     * @throws std::runtime_error If the rendererType is unknown.
     */
    static std::unique_ptr<Renderer> createRenderer(const RenderConfig& config);
};
