#pragma once

#include <memory>
#include <vector>

#include "Camera.hpp"
#include "CommandPool.hpp"
#include "CubeMapImage.hpp"
#include "Descriptor.hpp"
#include "Device.hpp"
#include "Draughts.hpp"
#include "LightingBufferObject.hpp"
#include "Object.hpp"
#include "TextureImage.hpp"
#include "UniformBufferObject.hpp"

class Scene
{
public:
    Scene(std::shared_ptr<Device> device, std::shared_ptr<CommandPool> commandPool)
    {
        _uniformBufferObject = std::make_shared<UniformBufferObject>(device, 0, VK_SHADER_STAGE_VERTEX_BIT);
        _lightingBufferObject = std::make_shared<LightingBufferObject>(device, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
        
        _cubeMapImage = std::make_shared<CubeMapImage>(device, commandPool, VK_FORMAT_R32_SFLOAT, 2, VK_SHADER_STAGE_FRAGMENT_BIT);
        
        _environmentMapImages.emplace_back(std::make_shared<CubeMapImage>(device, commandPool, VK_FORMAT_R32G32B32A32_SFLOAT, 9, VK_SHADER_STAGE_FRAGMENT_BIT));
        _environmentMapImages.emplace_back(std::make_shared<CubeMapImage>(device, commandPool, VK_FORMAT_R32G32B32A32_SFLOAT, 10, VK_SHADER_STAGE_FRAGMENT_BIT));
        _environmentMapImages.emplace_back(std::make_shared<CubeMapImage>(device, commandPool, VK_FORMAT_R32G32B32A32_SFLOAT, 11, VK_SHADER_STAGE_FRAGMENT_BIT));
        _environmentMapImages.emplace_back(std::make_shared<CubeMapImage>(device, commandPool, VK_FORMAT_R32G32B32A32_SFLOAT, 12, VK_SHADER_STAGE_FRAGMENT_BIT));
        
        _textureImages.emplace_back(std::make_shared<TextureImage>(device, commandPool, "textures/stones.jpg", 3, VK_SHADER_STAGE_FRAGMENT_BIT));
        _textureImages.emplace_back(std::make_shared<TextureImage>(device, commandPool, "textures/texture.jpg", 4, VK_SHADER_STAGE_FRAGMENT_BIT));
        _textureImages.emplace_back(std::make_shared<TextureImage>(device, commandPool, "textures/seamless-wood-texture-1.jpg", 5, VK_SHADER_STAGE_FRAGMENT_BIT));
        _textureImages.emplace_back(std::make_shared<TextureImage>(device, commandPool, "textures/dfdgag_largest.jpg", 6, VK_SHADER_STAGE_FRAGMENT_BIT));
        _textureImages.emplace_back(std::make_shared<TextureImage>(device, commandPool, "textures/wm_indoorwood44_1024.png", 7, VK_SHADER_STAGE_FRAGMENT_BIT));
        _textureImages.emplace_back(std::make_shared<TextureImage>(device, commandPool, "textures/wood.jpg", 8, VK_SHADER_STAGE_FRAGMENT_BIT));
        
        _objects.emplace_back(std::make_shared<Object>(device, commandPool, "objects/maze.obj", 0, 1));
        _objects.emplace_back(std::make_shared<Object>(device, commandPool, "objects/bars.obj", 1, 1));
        _objects.emplace_back(std::make_shared<Object>(device, commandPool, "objects/table_chairs.obj", 2, 1));
        _objects.emplace_back(std::make_shared<Object>(device, commandPool, "objects/walls.obj", 0, 1));
        
        _draughts = std::make_shared<Draughts>(device, commandPool);
        
        _sphere = std::make_shared<Object>(device, commandPool, "objects/sphere_smooth.obj", 10, 4);
        
        // Set different reflection effects
        _sphere->instanceData(0).fresnel = 0.05;
        _sphere->instanceData(1).fresnel = 0.3;
        _sphere->instanceData(2).fresnel = 1;
        _sphere->instanceData(3).fresnel = 1;
        
        // Set different colors
        _sphere->instanceData(0).color = glm::vec3(0.8, 0.5, 0.2);
        _sphere->instanceData(1).color = glm::vec3(0.2, 0.8, 0.9);
        _sphere->instanceData(2).color = glm::vec3(0.7, 0.1, 0.9);
        _sphere->instanceData(3).color = glm::vec3(0.95, 0.24, 0.1);
        
        _sphere->instanceData(0).refraction = 1.5; //0.05;
        _sphere->instanceData(1).refraction = 1.1; //0.3;
        _sphere->instanceData(2).refraction = 0.2; //0.7;
        _sphere->instanceData(3).refraction = 0.5; //0.95;
        
        // Initialize descriptors
        _descriptors.emplace_back(_uniformBufferObject->descriptor());
        _descriptors.emplace_back(_lightingBufferObject->descriptor());
        
        _descriptors.emplace_back(_cubeMapImage->descriptor());
        
        for (const auto& textureImage : _textureImages) {
            _descriptors.emplace_back(textureImage->descriptor());
        }
        
        for (const auto& environmentMapImage : _environmentMapImages) {
            _descriptors.emplace_back(environmentMapImage->descriptor());
        }
    }
    
    ~Scene()
    {
        
    }
    
    std::vector<std::shared_ptr<Object>> objects()
    {
        return _objects;
    }
    
    std::vector<std::shared_ptr<TextureImage>> textureImages()
    {
        return _textureImages;
    }
    
    std::shared_ptr<Draughts> draughts()
    {
        return _draughts;
    }
    
    std::shared_ptr<Object> sphere()
    {
        return _sphere;
    }
    
    void resetInstanceBufferObjects(size_t amountOfFramebuffers)
    {
        for (auto&& object : _objects) {
            object->resetInstanceBufferObject(amountOfFramebuffers);
        }
        
        _draughts->boardObject()->resetInstanceBufferObject(amountOfFramebuffers);
        _draughts->draughtsObject()->resetInstanceBufferObject(amountOfFramebuffers);
        
        _sphere->resetInstanceBufferObject(amountOfFramebuffers);
    }
    
    void updateInstanceBufferObjects(uint32_t currentImage)
    {
        for (auto&& object : _objects) {
            object->updateInstanceBufferObject(currentImage);
        }
        
        _draughts->boardObject()->updateInstanceBufferObject(currentImage);
        _draughts->draughtsObject()->updateInstanceBufferObject(currentImage);
        _draughts->update();
        
        _sphere->updateInstanceBufferObject(currentImage);
    }
    
    std::shared_ptr<UniformBufferObject> uniformBufferObject()
    {
        return _uniformBufferObject;
    }
    
    std::shared_ptr<LightingBufferObject> lightingBufferObject()
    {
        return _lightingBufferObject;
    }
    
    std::shared_ptr<CubeMapImage> cubeMapImage()
    {
        return _cubeMapImage;
    }
    
    std::vector<std::shared_ptr<CubeMapImage>> environmentMapImages()
    {
        return _environmentMapImages;
    }
    
    const std::vector<Descriptor>& descriptors()
    {
        return _descriptors;
    }
    
private:
    std::vector<Descriptor> _descriptors;
    
    std::shared_ptr<UniformBufferObject> _uniformBufferObject;
    std::shared_ptr<LightingBufferObject> _lightingBufferObject;
    
    std::shared_ptr<CubeMapImage> _cubeMapImage;
    std::vector<std::shared_ptr<CubeMapImage>> _environmentMapImages;
    
    std::vector<std::shared_ptr<Object>> _objects;
    std::vector<std::shared_ptr<TextureImage>> _textureImages;
    std::shared_ptr<Draughts> _draughts;
    std::shared_ptr<Object> _sphere;
};
