//

#define TINYOBJLOADER_IMPLEMENTATION

#include "ObjectLoader.h"


//
// Created by adria on 10/06/2024.

ObjectLoader::ObjectLoader() {

}

ObjectLoader::ObjectLoader(std::vector<ObjectInformation *> *listObjects,
                           std::vector <Vertex> *vertices,
                           std::vector <uint32_t> *indices) {
    this->listObjects = listObjects;
    this->vertices = vertices;
    this->indices = indices;
}

void ObjectLoader::loadModel(ObjectInformation* objectInformation, uint32_t index = 0) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    std::string modelPath = MODEL_PATH + objectInformation->modelPath;
    //std::string modelPathWaterMelon = "models/furniture/Laptop/SAMSUNG_Laptop.obj";
    //std::string modelPath1 = "models/viking_room.obj";

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str())) {
        throw std::runtime_error(warn + err);
    }

    //std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for (const auto& shape : shapes) {
        for (size_t i = 0; i < shape.mesh.indices.size(); i += 3) {
            tinyobj::index_t idx0 = shape.mesh.indices[i];
            tinyobj::index_t idx1 = shape.mesh.indices[i + 2];
            tinyobj::index_t idx2 = shape.mesh.indices[i + 1];

            glm::vec3 v0 = {
                    attrib.vertices[3 * idx0.vertex_index + 0],
                    attrib.vertices[3 * idx0.vertex_index + 2],
                    attrib.vertices[3 * idx0.vertex_index + 1]
            };

            glm::vec3 v1 = {
                    attrib.vertices[3 * idx1.vertex_index + 0],
                    attrib.vertices[3 * idx1.vertex_index + 2],
                    attrib.vertices[3 * idx1.vertex_index + 1]
            };

            glm::vec3 v2 = {
                    attrib.vertices[3 * idx2.vertex_index + 0],
                    attrib.vertices[3 * idx2.vertex_index + 2],
                    attrib.vertices[3 * idx2.vertex_index + 1]
            };

            glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

            Vertex vertex0{}, vertex1{}, vertex2{};

            vertex0.pos = v0;
            vertex1.pos = v1;
            vertex2.pos = v2;

            vertex0.texCoord = {
                    attrib.texcoords[2 * idx0.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * idx0.texcoord_index + 1]
            };

            vertex1.texCoord = {
                    attrib.texcoords[2 * idx1.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * idx1.texcoord_index + 1]
            };

            vertex2.texCoord = {
                    attrib.texcoords[2 * idx2.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * idx2.texcoord_index + 1]
            };

            vertex0.color = vertex1.color = vertex2.color = {1.0f, 0.0f, 1.0f};
            vertex0.normal = vertex1.normal = vertex2.normal = normal;
            vertex0.objectIndex = vertex1.objectIndex = vertex2.objectIndex = index;

            computeTangentAndBitangent(vertex0, vertex1, vertex2);

            if(objectInformation->hasNormalMap){
                vertex0.hasNormal = 1;
                vertex1.hasNormal = 1;
                vertex2.hasNormal = 1;
            }else{
                vertex0.hasNormal = 0;
                vertex1.hasNormal = 0;
                vertex2.hasNormal = 0;
            }

            objectInformation->vertices.push_back(vertex0);
            objectInformation->vertices.push_back(vertex1);
            objectInformation->vertices.push_back(vertex2);

            objectInformation->localIndices.push_back(i);
            objectInformation->localIndices.push_back(i+1);
            objectInformation->localIndices.push_back(i+2);

            /*
            if (uniqueVertices.count(vertex0) == 0) {
                uniqueVertices[vertex0] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex0);
            }
            if (uniqueVertices.count(vertex1) == 0) {
                uniqueVertices[vertex1] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex1);
            }
            if (uniqueVertices.count(vertex2) == 0) {
                uniqueVertices[vertex2] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex2);
            }

            indices.push_back(uniqueVertices[vertex0]);
            indices.push_back(uniqueVertices[vertex1]);
            indices.push_back(uniqueVertices[vertex2]);
            */
        }
    }

    std::cout << "Loaded: " + objectInformation->modelPath + "\n";

}

void ObjectLoader::loadAllElements(){

    for (int i = 0; i < listObjects->size(); ++i) {
        if(listObjects->at(i)->mustBeLoaded){
            loadModel(listObjects->at(i), i);
        }
    }
}

void ObjectLoader::fillVertexAndIndices(){

    int indicesCount = 0;

    for(ObjectInformation* objectInformation : *listObjects){

        // Adding all the vertices
        vertices->insert(vertices->end(), objectInformation->vertices.begin(), objectInformation->vertices.end());

        //Adding the mapped indices
        std::transform(objectInformation->localIndices.begin(),
                       objectInformation->localIndices.end(),
                       std::back_inserter(*indices),
                       [&indicesCount](uint32_t x){return x + indicesCount;});

        indicesCount += objectInformation->vertices.size();

        std::cout << "Locally Buffers: " + objectInformation->modelPath + " \n";
    }

}


void ObjectLoader::addObject(ObjectInformation* objectInformation, std::vector<std::string> & texturePaths, std::vector<std::string> & normalPaths, std::vector<ObjectInformation*> &listObjectInfos, std::vector<Vertex> & vertices, std::vector<uint32_t> & indices){

    if(objectInformation->mustBeLoaded){
        loadModel(objectInformation, texturePaths.size());
    }

    texturePaths.push_back(objectInformation->texturePath);
    normalPaths.push_back(objectInformation->normalPath);

    listObjectInfos.push_back(objectInformation);

    updateVerticesAndIndices(objectInformation, vertices, indices);
}

void ObjectLoader::updateVerticesAndIndices(ObjectInformation* objectInformation, std::vector<Vertex> & vertices, std::vector<uint32_t> & indices){

    int indicesCount = vertices.size();

    // Adding all the vertices
    vertices.insert(vertices.end(), objectInformation->vertices.begin(), objectInformation->vertices.end());

    //Adding the mapped indices
    std::transform(objectInformation->localIndices.begin(),
                   objectInformation->localIndices.end(),
                   std::back_inserter(indices),
                   [&indicesCount](uint32_t x){return x + indicesCount;});

    indicesCount += objectInformation->vertices.size();

    std::cout << "THE LAST: " + objectInformation->modelPath + " \n";
    std::cout << "THE LAST: " + objectInformation->modelPath + " \n";
    std::cout << "THE LAST: " + objectInformation->modelPath + " \n";
    std::cout << indices.back();
    std::cout << std::endl;


    std::cout << "Locally Buffers: " + objectInformation->modelPath + " \n";

}


// Function to calculate tangent and bitangent for a triangle
void ObjectLoader::computeTangentAndBitangent(Vertex& v0, Vertex& v1, Vertex& v2) {
    glm::vec3 edge1 = v1.pos - v0.pos;
    glm::vec3 edge2 = v2.pos - v0.pos;

    glm::vec2 deltaUV1 = v1.texCoord - v0.texCoord;
    glm::vec2 deltaUV2 = v2.texCoord - v0.texCoord;

    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    glm::vec3 tangent;
    tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

    glm::vec3 bitangent;
    bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

    // Normalize the tangent and bitangent
    v0.tangent = glm::normalize(tangent);
    v1.tangent = glm::normalize(tangent);
    v2.tangent = glm::normalize(tangent);

    v0.bitangent = glm::normalize(bitangent);
    v1.bitangent = glm::normalize(bitangent);
    v2.bitangent = glm::normalize(bitangent);
}

void ObjectLoader::transformVertex(std::vector<Vertex> & tempVertices, std::vector<skyBoxVertex> & verticesSB) {
    verticesSB.resize(tempVertices.size());

    std::transform(tempVertices.begin(), tempVertices.end(), verticesSB.begin(), [](const Vertex& x) {return skyBoxVertex{x.pos};});
}