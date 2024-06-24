//
// Created by adria on 10/06/2024.
//

#ifndef VULKANPROJECTVALICAALVARADO_OBJECTLOADER_H
#define VULKANPROJECTVALICAALVARADO_OBJECTLOADER_H

#include <tiny_obj_loader.h>
#include <tiny_gltf.h>

#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>


#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <utility>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <optional>
#include <set>
#include <unordered_map>
#include "VertexStructure.h"

const std::string MODEL_PATH = "models/";

enum class modelSize{
    TINY, SMALL, MEDIUM, BIG, GIGANTIC, GLTF
};

struct ObjectInformation{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> localIndices;

    //If the object is to be loaded
    bool mustBeLoaded{};
    std::string modelPath;

    glm::mat4 modelMatrix{};
    std::string texturePath;

    bool hasNormalMap{};
    std::string normalPath;

    bool isGltf{};

    modelSize modelSize;

    uint32_t indexPresentation;
    std::string presentationPath;




    ObjectInformation(){}

    // THIS IS THE CONSTRUCTOR FOR INSTANTIATING THE OBJECT
    ObjectInformation(
            std::string modelPath,
            glm::mat4 modelMatrix,
            const std::string& texturePath,
            bool isGltf
            ){
        vertices = {};
        localIndices = {};

        mustBeLoaded = true;
        this->modelPath = std::move(modelPath);
        this->texturePath = texturePath;

        this->modelMatrix = modelMatrix;
        hasNormalMap = false;
        normalPath = texturePath;

        modelSize = modelSize::MEDIUM;

        this->isGltf = isGltf;
    }

    void translateModel(float x, float y, float z);
    void scaleModel(float amount);
    void rotateModel(float degrees, char axis);

    // THIS IS THE CONSTRUCTOR FOR PRESENTING THE ADDING OBJECT

    ObjectInformation(
            std::string modelPath,
            glm::mat4 modelMatrix,
            const std::string& texturePath,
            bool isGltf,
            uint32_t indexPresentation,
            std::string presentationPath
    ){
        vertices = {};
        localIndices = {};

        mustBeLoaded = true;
        this->modelPath = std::move(modelPath);
        this->texturePath = texturePath;

        this->modelMatrix = modelMatrix;
        hasNormalMap = false;
        normalPath = texturePath;

        modelSize = modelSize::MEDIUM;

        this->isGltf = isGltf;

        this->indexPresentation = indexPresentation;
        this->presentationPath = presentationPath;
    }
};


class ObjectLoader {

private:
    std::vector<ObjectInformation *> * listObjects;
    std::vector<Vertex> * vertices;
    std::vector<uint32_t> * indices;

public:

    ObjectLoader();

    ObjectLoader(
            std::vector<ObjectInformation *> * listObjects,
            std::vector<Vertex> * vertices,
            std::vector<uint32_t> * indices
    );

    void loadModel(ObjectInformation* objectInformation, uint32_t index);
    void loadAllElements();
    void fillVertexAndIndices();

    void addObject(ObjectInformation* objectInformation, std::vector<std::string> & texturePaths, std::vector<std::string> & normalPaths, std::vector<ObjectInformation*> &listObjectInfos, std::vector<Vertex> & vertices, std::vector<uint32_t> & indices);
    void updateVerticesAndIndices(ObjectInformation* objectInformation, std::vector<Vertex> & vertices, std::vector<uint32_t> & indices);

    void computeTangentAndBitangent(Vertex& v0, Vertex& v1, Vertex& v2);

    void transformVertex(std::vector<Vertex> & tempVertices, std::vector<skyBoxVertex> & verticesSB);

    void loadGLTFModel(ObjectInformation* objectInformation, uint32_t index);

    void createLoadablesVector(std::vector<ObjectInformation> & loadables);

};

    void createLivingRoom(std::vector<ObjectInformation> &listActualObjectInfos, std::vector<ObjectInformation*> &listObjectInfos);
    void createKitchen(std::vector<ObjectInformation> &listActualObjectInfos, std::vector<ObjectInformation*> &listObjectInfos);
    void createEmptyRoom(std::vector<ObjectInformation> &listActualObjectInfos, std::vector<ObjectInformation*> &listObjectInfos,
                         bool insideFunction);


#endif //VULKANPROJECTVALICAALVARADO_OBJECTLOADER_H