#pragma once

#include <memory>

class FrameManager
{
public:
    static const int MAX_FRAMES_IN_FLIGHT = 2;
    
public:
    FrameManager();
    ~FrameManager();
    
    size_t current();
    void increment();
    
private:
    class Private;
    std::unique_ptr<Private> _delegate;
};
