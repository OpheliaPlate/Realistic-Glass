#pragma once

#include <memory>

#include <glm/glm.hpp>

class Window;

class Camera
{
public:
    Camera(std::shared_ptr<Window> window);
    ~Camera();
    
    glm::vec3 position();
    glm::mat4 view();
    
private:
    class Private;
    std::unique_ptr<Private> _delegate;
};
