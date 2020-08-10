#pragma once

#include <deque>

#include "InstanceBufferObject.hpp"

class Board
{
    struct Index
    {
        int8_t x;
        int8_t y;
        
        bool operator ==(const Index& other)
        {
            return x == other.x && y == other.y;
        }
        
        bool operator !=(const Index& other)
        {
            return x != other.x || y != other.y;
        }
    };
    
public:
    Board(InstanceBufferObject::Structure& instanceData)
        : _instanceData(instanceData)
    {
        _instanceData.selected = 0;
    }
    
    void addSelected(int8_t x, int8_t y) {
        _selected.emplace_back(Index{x, y});
        
        _instanceData.selected = 0;
        for (int i = 0; i < _selected.size(); ++i) {
            // Add 1 to the selected to always make sure we know it's a real value and not just zeros
            _instanceData.selected += _selected[i].y * std::pow(10, i*2 + 1) + (_selected[i].x+1) * std::pow(10, i*2);
        }
    }
    
    void resetSelected()
    {
        _selected.clear();
        _instanceData.selected = 0;
    }
    
private:
    std::vector<Index> _selected;
    InstanceBufferObject::Structure& _instanceData;
};
