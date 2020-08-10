#include "FrameManager.hpp"

class FrameManager::Private
{
public:
    Private()
    {
        
    }
    
    ~Private()
    {
        
    }
    
    size_t current()
    {
        return _currentFrame;
    }
    
    void increment()
    {
        _currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
    
private:
    size_t _currentFrame = 0;
};

FrameManager::FrameManager()
    : _delegate(std::make_unique<Private>())
{
    
}

FrameManager::~FrameManager()
{
    
}

size_t FrameManager::current()
{
    return _delegate->current();
}

void FrameManager::increment()
{
    _delegate->increment();
}
