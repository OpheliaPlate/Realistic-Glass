#pragma once

#include <chrono>
#include <deque>

#include "InstanceBufferObject.hpp"

class Draught
{
    static constexpr int32_t kAnimationDuration = 20;
    
public:
    enum class Color : uint8_t
    {
        White,
        Black
    };
    
    enum class State : uint8_t
    {
        Normal,
        Captured,
        Lady,
        Crown
    };
    
    struct Position
    {
        int8_t x;
        int8_t y;
        
        bool operator ==(const Position& other)
        {
            return x == other.x && y == other.y;
        }
        
        bool operator !=(const Position& other)
        {
            return x != other.x || y != other.y;
        }
    };
    
public:
    Draught(InstanceBufferObject::Structure& instanceData, Position position, Color color)
        : _instanceData(instanceData)
        , _positionQueue{position}
        , _color(color)
    {
        _instanceData.pos = glm::vec3(-0.135f + position.x * 0.03f,
                                      -0.135f + position.y * 0.03f,
                                      0.57f);
        
        _instanceData.texIndex = color == Color::White ? 4 : 5;
        _instanceData.selected = 0;
    }
    
    int32_t id() const
    {
        return _instanceData.id;
    }
    
    Position position() const
    {
        return _positionQueue.back();
    }
    
    void setPosition(const std::vector<Position>& nextPositions)
    {
        _positionQueue.insert(_positionQueue.end(), nextPositions.begin(), nextPositions.end());
        
        if (_state == State::Lady) {
            _crown->setPosition(nextPositions);
        }
    }
    
    Draught* crown()
    {
        return _crown;
    }
    
    void setCrown(Draught* crown)
    {
        crown->setLady(this);
        _crown = crown;
    }
    
    Draught* lady()
    {
        return _lady;
    }
    
    void setLady(Draught* lady)
    {
        _state = State::Crown;
        _lady = lady;
    }
    
    State state()
    {
        return _state;
    }
    
    void setState(State state)
    {
        _state = state;
    }
    
    bool selected()
    {
        return _state == State::Crown ? false : _instanceData.selected;
    }
    
    void setSelected(bool selected)
    {
        if (_state == State::Crown) {
            _lady->_instanceData.selected = selected;
        } else if (_state == State::Lady) {
            _crown->_instanceData.selected = selected;
        }
        
        _instanceData.selected = selected;
    }
    
    Color color() const
    {
        return _color;
    }
    
    bool animate()
    {
        if (_currentFrame > kAnimationDuration) {
            _positionQueue.pop_front();
            _currentFrame = 0;
        }
        
        if (_positionQueue.size() == 1) {
            return false;
        }
        
        auto prevPos = _positionQueue.front();
        auto nextPos = _positionQueue.at(1);
        
        // Ignore placeholder animation for getting captured for now
        if (prevPos == nextPos) {
            _currentFrame++;
            return true;
        }
        
        float prevX = -0.135f + prevPos.x * 0.03f;
        float prevY = -0.135f + prevPos.y * 0.03f;
        float nextX = -0.135f + nextPos.x * 0.03f;
        float nextY = -0.135f + nextPos.y * 0.03f;
        float halfAnimationDuration = kAnimationDuration / 2.0f;
        float jumpCurve = -0.5f;
        float jumpHeight = 0.02f;
        float x = prevX - ((prevX - nextX) / kAnimationDuration) * _currentFrame;
        float y = prevY - ((prevY - nextY) / kAnimationDuration) * _currentFrame;
        
        // Crowns float above the ladies
        float z = _state == State::Crown ? 0.578f : 0.57f;
        z += (jumpCurve * ((std::pow(_currentFrame - halfAnimationDuration, 2) / (std::pow(halfAnimationDuration, 2) / (jumpHeight / -jumpCurve)))) + jumpHeight);
        
        _instanceData.pos = glm::vec3(x, y, z);
        
        _currentFrame++;
        return true;
    }
    
private:
    std::deque<Position> _positionQueue;
    Color _color;
    State _state{State::Normal};
    int32_t _currentFrame{0};
    Draught* _crown{nullptr};
    Draught* _lady{nullptr};
    
    InstanceBufferObject::Structure& _instanceData;
};
