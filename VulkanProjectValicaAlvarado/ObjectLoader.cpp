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
    ObjectInformation objTurret {};
    objTurret.modelPath = "turret.obj";
    objTurret.texturePath = "";
    objTurret.mustBeLoaded = true;
    objTurret.modelMatrix = glm::mat4(1.0f);


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

