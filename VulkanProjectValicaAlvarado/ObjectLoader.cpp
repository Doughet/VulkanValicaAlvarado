//

#define TINYOBJLOADER_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION


#include "ObjectLoader.h"

//
// Created by adria on 10/06/2024.

void ObjectInformation::translateModel(float x, float y, float z) {
    this->modelMatrix = glm::translate(this->modelMatrix, glm::vec3(x, y, z));
}

void ObjectInformation::scaleModel(float amount) {
    this->modelMatrix = glm::scale(this->modelMatrix, glm::vec3(amount, amount, amount));
}

void ObjectInformation::rotateModel(float degrees, char axis){
    switch(axis){
        case 'x':
            this->modelMatrix = glm::rotate(this->modelMatrix, glm::radians(degrees) , glm::vec3(1.0f, 0.0f, 0.0f));
            break;
        case 'y':
            this->modelMatrix = glm::rotate(this->modelMatrix, glm::radians(degrees) , glm::vec3(0.0f, 1.0f, 0.0f));
            break;
        case 'z':
            this->modelMatrix = glm::rotate(this->modelMatrix, glm::radians(degrees) , glm::vec3(0.0f, 0.0f, 1.0f));
            break;
        default:
            printf("Specify one axis (x, y, z) for function: void rotateModel(float degrees, char axis)");
            break;
    }
}

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

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str(), "models/furniture/CoconutTree/")) {
        throw std::runtime_error(warn + err);
    }


    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

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

            /*
            objectInformation->vertices.push_back(vertex0);
            objectInformation->vertices.push_back(vertex1);
            objectInformation->vertices.push_back(vertex2);

            objectInformation->localIndices.push_back(i);
            objectInformation->localIndices.push_back(i+1);
            objectInformation->localIndices.push_back(i+2);
             */


            if (uniqueVertices.count(vertex0) == 0) {
                uniqueVertices[vertex0] = static_cast<uint32_t>(objectInformation->vertices.size());
                objectInformation->vertices.push_back(vertex0);
            }
            if (uniqueVertices.count(vertex1) == 0) {
                uniqueVertices[vertex1] = static_cast<uint32_t>(objectInformation->vertices.size());
                objectInformation->vertices.push_back(vertex1);
            }
            if (uniqueVertices.count(vertex2) == 0) {
                uniqueVertices[vertex2] = static_cast<uint32_t>(objectInformation->vertices.size());
                objectInformation->vertices.push_back(vertex2);
            }

            objectInformation->localIndices.push_back(uniqueVertices[vertex0]);
            objectInformation->localIndices.push_back(uniqueVertices[vertex1]);
            objectInformation->localIndices.push_back(uniqueVertices[vertex2]);

        }
    }

    std::cout << "Loaded: " + objectInformation->modelPath + "\n";

}

void ObjectLoader::loadAllElements(){

    for (int i = 0; i < listObjects->size(); ++i) {
        if(listObjects->at(i)->mustBeLoaded){

            if(listObjects->at(i)->isGltf){
                loadGLTFModel(listObjects->at(i), i);
            }else{
                loadModel(listObjects->at(i), i);
            }
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

        if(objectInformation->isGltf){
            loadGLTFModel(objectInformation, listObjectInfos.size());
        }else{
            loadModel(objectInformation, listObjectInfos.size());
        }
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

template<typename T>
void processAttribute(tinygltf::Model& model, const std::map<std::string, int>& attributes, const std::string& attributeName, int componentCount, std::vector<Vertex>& vertices, uint32_t index, bool hasNormalMap) {
    if (attributes.find(attributeName) != attributes.end()) {
        const auto& accessor = model.accessors[attributes.find(attributeName)->second];
        const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
        const T* bufferData = reinterpret_cast<const T*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);

        for (size_t i = 0; i < accessor.count; i++) {
            Vertex vertex;
            for (int j = 0; j < componentCount; j++) {
                // Depending on the component count, assign to correct vertex attribute
            }
            // Additional vertex setup
            vertices.push_back(vertex);
        }
    }
}


void ze(ObjectInformation* objectInformation, uint32_t index) {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    std::string modelPath = MODEL_PATH + objectInformation->modelPath;
    if (!loader.LoadASCIIFromFile(&model, &err, &warn, modelPath)) {
        throw std::runtime_error("Failed to load glTF file: " + warn + err);
    }

    std::cout << "Loaded: " + objectInformation->modelPath + "\n";

    // Process each mesh
    for (const auto& mesh : model.meshes) {
        for (const auto& primitive : mesh.primitives) {
            // Check and process each required attribute
            processAttribute<float>(model, primitive.attributes, "POSITION", 3, objectInformation->vertices, index, objectInformation->hasNormalMap);
            processAttribute<float>(model, primitive.attributes, "NORMAL", 3, objectInformation->vertices, index, objectInformation->hasNormalMap);
            processAttribute<float>(model, primitive.attributes, "TEXCOORD_0", 2, objectInformation->vertices, index, objectInformation->hasNormalMap);

            // Handle indices
            const auto& indexAccessor = model.accessors[primitive.indices];
            const tinygltf::BufferView& indexView = model.bufferViews[indexAccessor.bufferView];
            const tinygltf::Buffer& indexBuffer = model.buffers[indexView.buffer];
            const uint16_t* indices = reinterpret_cast<const uint16_t*>(&indexBuffer.data[indexAccessor.byteOffset + indexView.byteOffset]);

            for (size_t i = 0; i < indexAccessor.count; i++) {
                objectInformation->localIndices.push_back(indices[i]);
            }
        }
    }
}


// Function to load a GLTF model
void ObjectLoader::loadGLTFModel(ObjectInformation* objectInformation, uint32_t index) {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    std::string modelPath = MODEL_PATH + objectInformation->modelPath;
    if (!loader.LoadASCIIFromFile(&model, &err, &warn, modelPath.c_str())) {
        throw std::runtime_error(warn + err);
    }

    std::cout << "Loaded: " + objectInformation->modelPath + "\n";

    for (const auto& mesh : model.meshes) {
        for (const auto& primitive : mesh.primitives) {
            // Access vertex positions
            const auto& posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
            const tinygltf::BufferView& posView = model.bufferViews[posAccessor.bufferView];
            const tinygltf::Buffer& posBuffer = model.buffers[posView.buffer];
            const float* positions = reinterpret_cast<const float*>(&posBuffer.data[posAccessor.byteOffset + posView.byteOffset]);

            // Access normals
            const auto& normAccessor = model.accessors[primitive.attributes.find("NORMAL")->second];
            const tinygltf::BufferView& normView = model.bufferViews[normAccessor.bufferView];
            const tinygltf::Buffer& normBuffer = model.buffers[normView.buffer];
            const float* normals = reinterpret_cast<const float*>(&normBuffer.data[normAccessor.byteOffset + normView.byteOffset]);

            // Access texture coordinates
            const auto& texAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
            const tinygltf::BufferView& texView = model.bufferViews[texAccessor.bufferView];
            const tinygltf::Buffer& texBuffer = model.buffers[texView.buffer];
            const float* texCoords = reinterpret_cast<const float*>(&texBuffer.data[texAccessor.byteOffset + texView.byteOffset]);

            // Access indices
            const auto& indexAccessor = model.accessors[primitive.indices];
            const tinygltf::BufferView& indexView = model.bufferViews[indexAccessor.bufferView];
            const tinygltf::Buffer& indexBuffer = model.buffers[indexView.buffer];
            const uint32_t * indices = reinterpret_cast<const uint32_t *>(&indexBuffer.data[indexAccessor.byteOffset + indexView.byteOffset]);

            for (size_t i = 0; i < indexAccessor.count; i += 3) {
                uint16_t idx0 = indices[i];
                uint16_t idx1 = indices[i + 1];
                uint16_t idx2 = indices[i + 2];

                glm::vec3 v0 = { positions[3 * idx0 + 0], positions[3 * idx0 + 1], positions[3 * idx0 + 2] };
                glm::vec3 v1 = { positions[3 * idx1 + 0], positions[3 * idx1 + 1], positions[3 * idx1 + 2] };
                glm::vec3 v2 = { positions[3 * idx2 + 0], positions[3 * idx2 + 1], positions[3 * idx2 + 2] };

                glm::vec3 normal0 = { normals[3 * idx0 + 0], normals[3 * idx0 + 1], normals[3 * idx0 + 2] };
                glm::vec3 normal1 = { normals[3 * idx1 + 0], normals[3 * idx1 + 1], normals[3 * idx1 + 2] };
                glm::vec3 normal2 = { normals[3 * idx2 + 0], normals[3 * idx2 + 1], normals[3 * idx2 + 2] };

                glm::vec2 texCoord0 = { texCoords[2 * idx0 + 0], 1.0f - texCoords[2 * idx0 + 1] };
                glm::vec2 texCoord1 = { texCoords[2 * idx1 + 0], 1.0f - texCoords[2 * idx1 + 1] };
                glm::vec2 texCoord2 = { texCoords[2 * idx2 + 0], 1.0f - texCoords[2 * idx2 + 1] };

                glm::vec3 faceNormal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

                Vertex vertex0{}, vertex1{}, vertex2{};

                vertex0.pos = v0;
                vertex0.normal = normal0;
                vertex0.texCoord = texCoord0;
                vertex0.color = { 1.0f, 0.0f, 1.0f };
                vertex0.objectIndex = index;
                vertex0.hasNormal = objectInformation->hasNormalMap ? 1 : 0;

                vertex1.pos = v1;
                vertex1.normal = normal1;
                vertex1.texCoord = texCoord1;
                vertex1.color = { 1.0f, 0.0f, 1.0f };
                vertex1.objectIndex = index;
                vertex1.hasNormal = objectInformation->hasNormalMap ? 1 : 0;

                vertex2.pos = v2;
                vertex2.normal = normal2;
                vertex2.texCoord = texCoord2;
                vertex2.color = { 1.0f, 0.0f, 1.0f };
                vertex2.objectIndex = index;
                vertex2.hasNormal = objectInformation->hasNormalMap ? 1 : 0;

                computeTangentAndBitangent(vertex0, vertex1, vertex2);

                objectInformation->vertices.push_back(vertex0);
                objectInformation->vertices.push_back(vertex1);
                objectInformation->vertices.push_back(vertex2);

                objectInformation->localIndices.push_back(objectInformation->vertices.size() - 3);
                objectInformation->localIndices.push_back(objectInformation->vertices.size() - 2);
                objectInformation->localIndices.push_back(objectInformation->vertices.size() - 1);
            }
        }
    }
}



void ObjectLoader::createLoadablesVector(std::vector<ObjectInformation> & loadables){
    ObjectInformation objectCoconutTree = ObjectInformation(
            "furniture/CoconutTree/coconutTree.obj",
            glm::mat4(1.0f),
            "furniture/CoconutTree/coconutTreeTexture.jpg",
            false,
            0,
            "presentations/furniture/CoconutTree/coconutPresentation.png"
            );

    ObjectInformation objectLaptop = ObjectInformation(
            "furniture/Laptop/SAMSUNG_Laptop.obj",
            glm::mat4(1.0f),
            "furniture/Laptop/SLT_Dif.png",
            false,
            1,
            "presentations/furniture/CoconutTree/coconutPresentation.png"
    );

    ObjectInformation objectCat = ObjectInformation(
            "animals/cat/Cat.obj",
            glm::mat4(1.0f),
            "animals/cat/Cat_diffuse.jpeg",
            false,
            2,
            "presentations/animals/cat/catPresentation.png"
    );
    objectCat.rotateModel(-90, 'x');



    ObjectInformation objSofa = ObjectInformation(
            "furniture/Sofa/scene.gltf",
            glm::scale(glm::mat4(1.0f), glm::vec3(13.0f)),
            "furniture/Sofa/Material_baseColor.jpeg",
            true,
            3,
            "presentations/furniture/Sofa/ChairPresentation.png"
    );
    objSofa.scaleModel(15.0f);
    objSofa.translateModel(0.0f, 0.0f, 0.1f);
    objSofa.rotateModel(-83, 'z');

    ObjectInformation objPiano = ObjectInformation(
            "furniture/PianoRemake/pianoRemake.obj",
            glm::scale(glm::mat4(1.0f), glm::vec3(13.0f)),
            "furniture/PianoRemake/BakingDiffuse.png",
            false,
            3,
            "presentations/furniture/PianoRemake/PianoRemakePresentation.png"
    );
    objPiano.scaleModel(15.0f);
    objPiano.translateModel(0.0f, 0.0f, 0.1f);
    objPiano.rotateModel(-83, 'z');


    ObjectInformation objCarpet = ObjectInformation(
            "furniture/Carpet/round-carpet.obj",
            glm::scale(glm::mat4(1.0f), glm::vec3(13.0f)),
            "furniture/Carpet/red_carpet.jpg",
            false,
            3,
            "presentations/furniture/Carpet/CarpetPresentation.png"
    );

    ObjectInformation objChest = ObjectInformation(
            "furniture/AccentChestFree/scene.gltf",
            glm::scale(glm::mat4(1.0f), glm::vec3(13.0f)),
            "furniture/AccentChestFree/accent_cabinet_Mat_baseColor.jpeg",
            true,
            3,
            "presentations/furniture/Carpet/CarpetPresentation.png"
    );
    objChest.scaleModel(15.0f);
    objChest.rotateModel(90, 'x');
    objChest.translateModel(0.0f, 0.0f, 0.1f);


    ObjectInformation objBamboo = ObjectInformation(
            "furniture/Bamboo/bamboo.obj",
            glm::scale(glm::mat4(1.0f), glm::vec3(1.0f)),
            "furniture/Bamboo/bambooTexture.jpeg",
            false,
            3,
            "presentations/furniture/Bamboo/BambooPresentation.png"
    );

    objBamboo.scaleModel(1.0f);
    objBamboo.translateModel(0.0f, 0.0f, 0.1f);
    objBamboo.rotateModel(-83, 'z');



    ObjectInformation objChair = ObjectInformation(
            "furniture/Chair/scene.gltf",
            glm::scale(glm::mat4(1.0f), glm::vec3(13.0f)),
            "furniture/Chair/Scene_-_Root_baseColor.png",
            true,
            3,
            "presentations/furniture/Chair/ChairPresentation.png"
    );

    objChair.scaleModel(15.0f);
    objChair.translateModel(0.0f, 0.0f, 0.1f);
    objChair.rotateModel(-83, 'z');


    ObjectInformation objChairKitchen = ObjectInformation(
            "furniture/ClassicRoundTable/scene.gltf",
            glm::scale(glm::mat4(1.0f), glm::vec3(13.0f)),
            "furniture/ClassicRoundTable/plywood-cc0.001_baseColor.jpeg",
            true,
            3,
            "presentations/furniture/ClassicRoundTable/RoundTablePresentation.png"
    );

    objChairKitchen.scaleModel(15.0f);
    objChairKitchen.translateModel(0.0f, 0.0f, 0.1f);
    objChairKitchen.rotateModel(-90, 'x');

    ObjectInformation objConsoleTable = ObjectInformation(
            "furniture/ConsoleTable/scene.gltf",
            glm::scale(glm::mat4(1.0f), glm::vec3(13.0f)),
            "furniture/ConsoleTable/ConsoleTable_baseColor.png",
            true,
            3,
            "presentations/furniture/ConsoleTable/ConsoleTablePresentation.png"
    );

    objConsoleTable.scaleModel(15.0f);
    objConsoleTable.translateModel(0.0f, 0.0f, 0.1f);
    objConsoleTable.rotateModel(-83, 'z');



    ObjectInformation objDinningCar = ObjectInformation(
            "furniture/DiningCar/scene.gltf",
            glm::scale(glm::mat4(1.0f), glm::vec3(13.0f)),
            "furniture/DiningCar/Storage_Cart_Metal_baseColor.png",
            true,
            3,
            "presentations/furniture/DiningCar/DiningCarPresentation.png"
    );

    objDinningCar.scaleModel(15.0f);
    objDinningCar.translateModel(0.0f, 0.0f, 0.1f);
    objDinningCar.rotateModel(-83, 'z');

    ObjectInformation objGardeFaucet = ObjectInformation(
            "furniture/GardenHouseFaucet/scene.gltf",
            glm::scale(glm::mat4(1.0f), glm::vec3(13.0f)),
            "furniture/GardenHouseFaucet/Scene_-_Root_baseColor.png",
            true,
            3,
            "presentations/furniture/GardenFaucet/GardenFaucetPresentation.png"
    );

    objGardeFaucet.scaleModel(1.0f);
    objGardeFaucet.translateModel(0.0f, 0.0f, 0.1f);
    objGardeFaucet.rotateModel(-83, 'z');


    ObjectInformation objHedge = ObjectInformation(
            "furniture/Hedge/hedgeTextured.obj",
            glm::scale(glm::mat4(1.0f), glm::vec3(6.0f)),
            "furniture/Hedge/hedge-displacement-texture.jpeg",
            false,
            3,
            "presentations/furniture/Hedge/HedgePresentation.png"
    );

    objHedge.scaleModel(1.0f);
    objHedge.translateModel(0.0f, 0.0f, 0.1f);
    objHedge.rotateModel(-83, 'z');



    ObjectInformation objKitchenFridge = ObjectInformation(
            "furniture/KitchenFridge/scene.gltf",
            glm::scale(glm::mat4(1.0f), glm::vec3(13.0f)),
            "furniture/KitchenFridge/baked_baseColor.png",
            true,
            3,
            "presentations/furniture/KitchenFridge/KitchenFridgePresentation.png"
    );

    objKitchenFridge.scaleModel(15.0f);
    objKitchenFridge.translateModel(0.0f, 0.0f, 0.1f);
    objKitchenFridge.rotateModel(-83, 'z');


    ObjectInformation objKitchenSink = ObjectInformation(
            "furniture/KitchenSink/scene.gltf",
            glm::scale(glm::mat4(1.0f), glm::vec3(13.0f)),
            "furniture/KitchenSink/cabinet_baseColor.png",
            true,
            3,
            "presentations/furniture/KitchenSink/KitchenSinkPresentation.png"
    );
    objKitchenSink.scaleModel(6.0f);
    objKitchenSink.translateModel(0.0f, 0.0f, 0.1f);
    objKitchenSink.rotateModel(-83, 'z');

    ObjectInformation objMorrisChair = ObjectInformation(
            "furniture/MorrisChairRemake/untitled.obj",
            glm::scale(glm::mat4(1.0f), glm::vec3(13.0f)),
            "furniture/MorrisChairRemake/bakingMorris.png",
            false,
            3,
            "presentations/furniture/MorrisChairRemake/MorrisChairRemakePresentation.png"
    );

    objMorrisChair.scaleModel(15.0f);
    objMorrisChair.translateModel(0.0f, 0.0f, 0.1f);
    objMorrisChair.rotateModel(-83, 'z');



    loadables.push_back(objectCoconutTree);
    loadables.push_back(objectCat);
    loadables.push_back(objSofa);
    loadables.push_back(objPiano);
    loadables.push_back(objCarpet);
    loadables.push_back(objBamboo);
    loadables.push_back(objChair);
    loadables.push_back(objChairKitchen);
    loadables.push_back(objConsoleTable);
    loadables.push_back(objDinningCar);
    loadables.push_back(objGardeFaucet);
    loadables.push_back(objHedge);
    loadables.push_back(objKitchenFridge);
    loadables.push_back(objKitchenSink);
    loadables.push_back(objMorrisChair);
}

void createEmptyRoom(std::vector<ObjectInformation> &listActualObjectInfos, std::vector<ObjectInformation*> &listObjectInfos,
                     bool insideFunction){

    ObjectInformation objWall1 = ObjectInformation(
            "furniture/Wall/wall.obj",
            glm::scale(glm::mat4(1.0f), glm::vec3(155.0f, 20.0f, 150.0f)),
            "furniture/Wall/wall_4.jpg",
            false);
    objWall1.rotateModel(180.0f, 'x');
    objWall1.translateModel(2.2f, -60.0f,  -4.75f);

    ObjectInformation objWall2 = ObjectInformation(
            "furniture/Wall/wall.obj",
            glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f)),
            "furniture/Wall/wall_4.jpg",
            false);
    objWall2.rotateModel(90, 'z');
    objWall2.modelMatrix = glm::scale(objWall2.modelMatrix, glm::vec3(110.0f, 20.0f, 150.0f));
    objWall2.translateModel(2.2f, -83.5f, -1.3f);

    ObjectInformation floor = ObjectInformation(
            "furniture/CyberpunkFlooring/scene.gltf",
            glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f)),
            "furniture/CyberpunkFlooring/wood_livingroom.jpg",
            true);
    floor.translateModel(0.0f, 0.0f, -96.0f);

    ObjectInformation room = ObjectInformation(
            "furniture/LivingRoom/livingRoomObject.obj",
            glm::scale(glm::mat4(1.0f), glm::vec3(500, 400, 300)),
            "furniture/LivingRoom/wood_livingroom.jpg",
            false);
    room.translateModel(0.33, -0.048, -0.36);


    /*listActualObjectInfos.push_back(floor);
    listActualObjectInfos.push_back(objWall1);
    listActualObjectInfos.push_back(objWall2);*/
    listActualObjectInfos.push_back(room);

    if(!insideFunction){
        /*listObjectInfos.push_back(&listActualObjectInfos[0]);
        listObjectInfos.push_back(&listActualObjectInfos[1]);
        listObjectInfos.push_back(&listActualObjectInfos[2]);*/
        listObjectInfos.push_back(&listActualObjectInfos[3]);
    }
}

void createKitchen(std::vector<ObjectInformation> &listActualObjectInfos, std::vector<ObjectInformation*> &listObjectInfos){
//createEmptyRoom(listActualObjectInfos, listObjectInfos, true);

    ObjectInformation objBaseKitchen = ObjectInformation("furniture/BlenderKitchen/PerfectKitchen.obj",
            glm::scale(glm::mat4(1.0f), glm::vec3(60.0f)),
            "BlenderKitchen/KitchenFlooring.jpg",
            false);

    ObjectInformation objFridge = ObjectInformation("furniture/KitchenFridge/scene.gltf",
                                                         glm::scale(glm::mat4(1.0f), glm::vec3(60.0f)),
                                                         "furniture/KitchenFridge/baked_baseColor.png",
                                                         true);//, 1, "presentation/furniture/FridgeKitchen/FridgeKitchen.png");
    objFridge.rotateModel(180, 'x');
    objFridge.rotateModel(180, 'y');

//    objFridge.translateModel(-3.0f, 3.5f, 0.0f);
    objFridge.translateModel(291.74f / 60, -155.40f / 60, 0.0f);

    ObjectInformation objPlant = ObjectInformation("furniture/BananaPlantWithPot/scene.gltf",
                                                    glm::scale(glm::mat4(1.0f), glm::vec3(5.0f)),
                                                    "furniture/BananaPlantPot/material_3_baseColor.jpeg",
                                                    true, 1, "presentations/furniture/BananaPlantWithPot/BananaPlantWithPot.png");
    objPlant.rotateModel(180, 'x');

   ObjectInformation objSink = ObjectInformation("furniture/KitchenSink/scene.gltf",
                                                   glm::scale(glm::mat4(1.0f), glm::vec3(25.0f)),
                                                   "furniture/KitchenSink/cabinet_baseColor.png",
                                                   true, 1, "presentations/furniture/KitchenSink/KitchenSink.png");
    objSink.rotateModel(-180, 'z');
    objSink.translateModel(402.74f / 25, -96.40f / 25, 1.42f / 25);
    ObjectInformation objDog = ObjectInformation("animals/dog/Dog.obj",
                                                  glm::scale(glm::mat4(1.0f), glm::vec3(1.2f)),
                                                  "animals/dog/Dog_diffuse.jpeg",
                                                  false/*, 1, "presentations/animals/KitchenSink/KitchenSink.png"*/);
    objDog.rotateModel(90, 'x');
    objDog.translateModel(0.0f, 0.0, 0.9);

    ObjectInformation objWineCoolerFridge = ObjectInformation("furniture/WineCoolerFridge/scene.gltf",
                                                 glm::scale(glm::mat4(1.0f), glm::vec3(100.0f)),
                                                 "furniture/WineCoolerFridge/Material.001_baseColor.png",
                                                 true/*, 1, "presentations/animals/KitchenSink/KitchenSink.png"*/);
   //objWineCoolerFridge.rotateModel(180, 'x');
    //objWineCoolerFridge.rotateModel(180, 'y');
  // objWineCoolerFridge.translateModel(-5.0f, 5.0f, -0.4f);

  /*  ObjectInformation objHangingLights = ObjectInformation("furniture/RusticHangingKitchenLights/scene.gltf",
                                                              glm::scale(glm::mat4(1.0f), glm::vec3(100.0f)),
                                                              "furniture/RusticHangingKitchenLights/bolts.002_baseColor.jpeg",
                                                              true);*/
    ObjectInformation objTable = ObjectInformation("furniture/OrnamentalWoodenTable/RoundTable.obj",
                                                           glm::scale(glm::mat4(1.0f), glm::vec3(50.0f)),
                                                           "furniture/OrnamentalWoodenDiningTable/Table_Leg_Wood_BaseColor.png",
                                                           false);

    ObjectInformation objWatermelon = ObjectInformation("furniture/Watermelon/watermelon_full n half.obj",
                                                   glm::scale(glm::mat4(1.0f), glm::vec3(50.0f)),
                                                   "furniture/Watermelon/watermelon_full n half-TM_u0_v0.png",
                                                   false);
    ObjectInformation objToiletPaper = ObjectInformation("furniture/ToiletPaper/scene.gltf",
                                                        glm::scale(glm::mat4(1.0f), glm::vec3(50.0f)),
                                                        "furniture/toiletScottex/DefaultMaterial_baseColor.jpeg",
                                                        true);
    ObjectInformation objToaster = ObjectInformation("furniture/Toaster/scene.gltf",
                                                         glm::scale(glm::mat4(1.0f), glm::vec3(50.0f)),
                                                         "furniture/Toaster/toaster_baseColor.png",
                                                         true);
    ObjectInformation objStoneTiger = ObjectInformation("furniture/StoneTiger/scene.gltf",
                                                     glm::scale(glm::mat4(1.0f), glm::vec3(50.0f)),
                                                     "furniture/StoneTiger/Archmodels_64_040__0_baseColor.jpeg",
                                                     true);
    ObjectInformation objLaptop = ObjectInformation("furniture/Laptop/SAMSUNG_Laptop.obj",
                                                        glm::scale(glm::mat4(1.0f), glm::vec3(50.0f)),
                                                        "furniture/Laptop/SLT_Dif.png",
                                                        false);
    ObjectInformation objChair = ObjectInformation("furniture/ChairKitchen/scene.gltf",
                                                    glm::scale(glm::mat4(1.0f), glm::vec3(50.0f)),
                                                    "furniture/ChairKitchen/WoodenTexture.jpg",
                                                    true);
    ObjectInformation objBin = ObjectInformation("furniture/KitchenBin/scene.gltf",
                                                   glm::scale(glm::mat4(1.0f), glm::vec3(50.0f)),
                                                   "furniture/KitchenBin/T_bin_baseColor.png",
                                                   true);
    listActualObjectInfos.push_back(objBaseKitchen);
    listActualObjectInfos.push_back(objFridge);
    //listActualObjectInfos.push_back(objPlant);
    listActualObjectInfos.push_back(objSink);
    listActualObjectInfos.push_back(objDog);
    listActualObjectInfos.push_back(objWineCoolerFridge);
   // listActualObjectInfos.push_back(objHangingLights);
    listActualObjectInfos.push_back(objTable);
    listActualObjectInfos.push_back(objToiletPaper);
    listActualObjectInfos.push_back(objToaster);
    listActualObjectInfos.push_back(objWatermelon);
    //listActualObjectInfos.push_back(objStoneTiger);
    //listActualObjectInfos.push_back(objLaptop);
    //listActualObjectInfos.push_back(objChair);
    //listActualObjectInfos.push_back(objBin);



    for(int i = 0; i < listActualObjectInfos.size(); i++){
        listObjectInfos.push_back(&listActualObjectInfos[i]);
    }
}

void createLivingRoom(std::vector<ObjectInformation> &listActualObjectInfos, std::vector<ObjectInformation*> &listObjectInfos){
    createEmptyRoom(listActualObjectInfos, listObjectInfos, true);
   /* ObjectInformation objMorris = ObjectInformation(
            "furniture/MorrisChair/morrisChair.obj",
            glm::scale(glm::mat4(1.0f), glm::vec3(3.0f)),
            "furniture/MorrisChair/morrisChair_smallChairMat_BaseColor.tga.png",
            false);
*/
    ObjectInformation objCarpet = ObjectInformation(
            "furniture/Carpet/round-carpet.obj",
            glm::scale(glm::mat4(1.0f), glm::vec3(3.0f)),
            "furniture/Carpet/red_carpet.jpg",
            false);
    objCarpet.scaleModel(3.1f);
    objCarpet.translateModel(0.0f, 0.0f, -7.5f);

    ObjectInformation objWildWestPiano = ObjectInformation(
            "furniture/PianoRemake/pianoRemake.obj",
            glm::scale(glm::mat4(1.0f), glm::vec3(200.0f)),
            "furniture/PianoRemake/BakingDiffuse.png",
            false);
    objWildWestPiano.translateModel(7.8f, 0.0f, -0.4f);
    objWildWestPiano.rotateModel(90.0f, 'z');

    ObjectInformation objTVCabinet = ObjectInformation(
            "furniture/TVCabinet/Tes.obj",
            glm::scale(glm::mat4(1.0f), glm::vec3(0.55f)),
            "furniture/TVCabinet/TCA_Dif.png",
            false);
    objTVCabinet.translateModel(-455.0f, 2000.0f, -180.0f);
    objTVCabinet.rotateModel(90.0f, 'x');

    ObjectInformation objCat = ObjectInformation(
            "animals/cat/Cat.obj",
            glm::scale(glm::mat4(1.0f), glm::vec3(3.0f)),
            "animals/cat/Cat_diffuse.jpeg",
            false);

    objCat.translateModel(110.0f, 20.0f, -24.0);
    objCat.rotateModel(90.0f, 'x');
    objCat.rotateModel(15.0f, 'y');
/*
    ObjectInformation objPlant1 = ObjectInformation(
            "furniture/PlantAsset/scene.gltf",
            glm::scale(glm::mat4(1.0f), glm::vec3(6.0f)),
            "furniture/PlantAsset/Material.002_baseColor.jpeg",
            true);
*/
  /*  ObjectInformation objVintageLamp = ObjectInformation(
            "furniture/VintageLamp/scene.gltf",
            glm::scale(glm::mat4(1.0f), glm::vec3(9.0f)),
            "furniture/VintageLamp/Lamp_baseColor.png",
            true);

    objVintageLamp.modelMatrix[1][1] *= -1;
    objVintageLamp.translateModel(15.0f, 15.0f, 12.0f);
    objVintageLamp.rotateModel(90, 'x');
*/
    ObjectInformation objSofa = ObjectInformation(
            "furniture/Sofa/scene.gltf",
            glm::scale(glm::mat4(1.0f), glm::vec3(13.0f)),
            "furniture/Sofa/Material_baseColor.jpeg",
            true);

    objSofa.scaleModel(15.0f);
    objSofa.translateModel(0.0f, 0.0f, 0.1f);
    objSofa.rotateModel(-83, 'z');


    listActualObjectInfos.push_back(objWildWestPiano);
    listActualObjectInfos.push_back(objTVCabinet);
    listActualObjectInfos.push_back(objCarpet);
    listActualObjectInfos.push_back(objCat);
    //listActualObjectInfos.push_back(objMorris);
    //listActualObjectInfos.push_back(objVintageLamp);
    listActualObjectInfos.push_back(objSofa);
    //listActualObjectInfos.push_back(objTable);

    for(int i = 0; i < listActualObjectInfos.size(); i++){
        listObjectInfos.push_back(&listActualObjectInfos[i]);
    }
}