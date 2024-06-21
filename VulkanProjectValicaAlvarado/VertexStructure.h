#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <vulkan/vulkan.h>

#ifndef VULKANPROJECTVALICAALVARADO_VERTEXSTRUCTURE_H
#define VULKANPROJECTVALICAALVARADO_VERTEXSTRUCTURE_H

#endif //VULKANPROJECTVALICAALVARADO_VERTEXSTRUCTURE_H


struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;
    glm::vec3 normal; // Add normal vector
    glm::int32 objectIndex;
    int8_t hasNormal;
    glm::vec3 tangent;
    glm::vec3 bitangent;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 8> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 8> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT; // Add format for normal
        attributeDescriptions[3].offset = offsetof(Vertex, normal);

        attributeDescriptions[4].binding = 0;
        attributeDescriptions[4].location = 4;
        attributeDescriptions[4].format = VK_FORMAT_R32_SINT; // Add format for index
        attributeDescriptions[4].offset = offsetof(Vertex, objectIndex);

        attributeDescriptions[5].binding = 0;
        attributeDescriptions[5].location = 5;
        attributeDescriptions[5].format = VK_FORMAT_R32_SINT; // Add format for index
        attributeDescriptions[5].offset = offsetof(Vertex, hasNormal);

        attributeDescriptions[6].binding = 0;
        attributeDescriptions[6].location = 6;
        attributeDescriptions[6].format = VK_FORMAT_R32G32B32_SFLOAT; // Add format for index
        attributeDescriptions[6].offset = offsetof(Vertex, tangent);

        attributeDescriptions[7].binding = 0;
        attributeDescriptions[7].location = 7;
        attributeDescriptions[7].format = VK_FORMAT_R32G32B32_SFLOAT; // Add format for index
        attributeDescriptions[7].offset = offsetof(Vertex, bitangent);

        return attributeDescriptions;
    }

    /**
     * @brief == operator that determines whether two vertexes are equal or not by comparing the different aattributes.
     * @param other The second vertex to be compared to.
     * @return True if the vertexes are equal and false otherwise
     */
    bool operator==(const Vertex& other) const {
        return pos == other.pos && color == other.color && texCoord == other.texCoord && normal == other.normal && objectIndex == other.objectIndex;
    }
};

namespace std {
    template<>
    struct hash<Vertex> {
        size_t operator()(const Vertex& vertex) const {
            return hash<glm::vec3>()(vertex.pos) ^
                   (hash<glm::vec3>()(vertex.color) << 1) ^
                   (hash<glm::vec2>()(vertex.texCoord) << 1) ^
                   (hash<glm::vec3>()(vertex.normal) << 1) ^
                   (hash<glm::int32>()(vertex.objectIndex) << 1) ^
                   (hash<int8_t>()(vertex.hasNormal) << 1) ^
                   (hash<glm::vec3>()(vertex.tangent) << 1) ^
                   (hash<glm::vec3>()(vertex.bitangent) << 1);
        }
    };
}

struct skyBoxVertex {
    glm::vec3 pos;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(skyBoxVertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 1> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 1> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(skyBoxVertex, pos);

        return attributeDescriptions;
    }

};