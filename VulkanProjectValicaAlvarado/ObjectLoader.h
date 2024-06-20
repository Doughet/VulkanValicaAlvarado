//
// Created by adria on 10/06/2024.
//

#ifndef VULKANPROJECTVALICAALVARADO_OBJECTLOADER_H
#define VULKANPROJECTVALICAALVARADO_OBJECTLOADER_H

#include <tiny_obj_loader.h>

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

struct ObjectInformation{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> localIndices;

    //If the object is to be loaded
    bool mustBeLoaded;
    std::string modelPath;

    glm::mat4 modelMatrix;
    std::string texturePath;

    bool hasNormalMap;
    std::string normalPath;
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
};


#endif //VULKANPROJECTVALICAALVARADO_OBJECTLOADER_H
