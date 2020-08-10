#include "Camera.hpp"

#include "Window.hpp"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Camera::Private
{
    static constexpr float kMaxVerticalAngle = 0.5f;
    
public:
    Private(std::shared_ptr<Window> window)
    {
        window->registerKeyCallback(GLFW_KEY_W, [this]() {
            _position += glm::vec3(glm::translate(_direction, glm::vec3(0.0f, 0.05f, 0.0f))[3]);
        });
        
        window->registerKeyCallback(GLFW_KEY_S, [this]() {
            _position += glm::vec3(glm::translate(_direction, glm::vec3(0.0f, -0.05f, 0.0f))[3]);
        });
        
        window->registerKeyCallback(GLFW_KEY_A, [this]() {
            _position += glm::vec3(glm::translate(_direction, glm::vec3(-0.05f, 0.0f, 0.0f))[3]);
        });
        
        window->registerKeyCallback(GLFW_KEY_D, [this]() {
            _position += glm::vec3(glm::translate(_direction, glm::vec3(0.05f, 0.0f, 0.0f))[3]);
        });
        
        window->registerKeyCallback(GLFW_KEY_UP, [this]() {
            _verticalAngle = std::min(kMaxVerticalAngle, _verticalAngle + 0.02f);
        });
        
        window->registerKeyCallback(GLFW_KEY_DOWN, [this]() {
            _verticalAngle = std::max(-kMaxVerticalAngle, _verticalAngle - 0.02f);
        });
        
        window->registerKeyCallback(GLFW_KEY_LEFT, [this]() {
            _horizontalAngle += 0.02f;
        });
        
        window->registerKeyCallback(GLFW_KEY_RIGHT, [this]() {
            _horizontalAngle -= 0.02f;
        });
        /*
        window->registerMouseMoveCallback([this](float xDelta, float yDelta) {
            _horizontalAngle -= xDelta / 100.0f;
            _verticalAngle = std::min(kMaxVerticalAngle, std::max(-kMaxVerticalAngle, _verticalAngle - yDelta / 100.0f));
        });
         */
         
    }
    
    ~Private()
    {
        
    }
    
    glm::vec3 position()
    {
        return _position;
    }

    glm::mat4 view()
    {
        // Update direction matrix for future translations
        _direction = glm::rotate(glm::mat4(1.0f), _horizontalAngle * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        auto viewCenter = glm::rotate(_direction, _verticalAngle * glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        
        glm::vec3 direction = _position + glm::vec3(glm::translate(viewCenter, glm::vec3(0.0f, 1.0f, 0.0f))[3]);
        return glm::lookAt(_position, direction, glm::vec3(0.0f, 0.0f, 1.0f));
    }
    
private:
    glm::vec3 _position{0.0f, -2.0f, 0.9f};
    bool _walking{false};
    
    float _horizontalAngle{0.0f};
    float _verticalAngle{0.0f};
    glm::mat4 _direction;
};

Camera::Camera(std::shared_ptr<Window> window)
    : _delegate(std::make_unique<Private>(window))
{
    
}

Camera::~Camera()
{
    
}

glm::vec3 Camera::position()
{
    return _delegate->position();
}

glm::mat4 Camera::view()
{
    return _delegate->view();
}
