#include "RendererFactory.hpp"
#include "FeatherRenderer.hpp"
#include "CollatzTreeRenderer.hpp"
#include <stdexcept>

std::unique_ptr<Renderer> RendererFactory::createRenderer(const RenderConfig& config) {
    if (config.rendererType == "feather") {
        return std::make_unique<FeatherRenderer>();
    } else if (config.rendererType == "tree") {
        return std::make_unique<CollatzTreeRenderer>();
    } else {
        throw std::runtime_error("Unknown renderer type: " + config.rendererType);
    }
}
