#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
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

#include "controller.h"

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

struct LightsBufferObject {
    glm::vec3 lightPos;
    glm::vec3 viewPos;
    glm::vec3 lightColor;
    glm::vec3 ambientColor;
    glm::vec3 diffuseColor;
    glm::vec3 specularColor;
    float shininess;
};

struct MatrixBufferObject{
    glm::mat4 model[20];
};

UniformBufferObject ubo{};

/*
void createFramebuffers() {
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        std::array<VkImageView, 3> attachments = {
                colorImageView,
                depthImageView,
                swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}
*/

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice &physicalDevice) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void updateMatrixUniformBuffer(uint32_t currentImage, std::vector<ObjectInformation> listActualObjectInfos,
                               std::vector<void*> &matrixBufferMapped){
    MatrixBufferObject mubo{};


    for (size_t i = 0; i < listActualObjectInfos.size(); ++i) {
        mubo.model[i] = listActualObjectInfos.at(i).modelMatrix;
    }

    /*
    mubo.model[0] = glm::scale(glm::mat4(1.0f), glm::vec3(15.0f, 15.0f, 15.0f));
    mubo.model[1] = glm::translate(glm::mat4(1.0f), glm::vec3(60, 0, 0));
    mubo.model[2] = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 1.50f, 12.0f));
*/

    //mubo.model[0] = listObjectInfos.at(0)->modelMatrix;

    memcpy(matrixBufferMapped[currentImage], &mubo, sizeof(mubo));
}

void updateUniformBuffer(uint32_t currentImage, GLFWwindow * &window,
                         std::vector<void*> &uniformBuffersMapped, std::vector<void*> &lightsBuffersMapped) {
    static bool debounce = false;
    static int curDebounce = 0;

    float deltaT;
    glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
    bool fire = false;
    getSixAxis(deltaT, m, r, fire, window);

    static float autoTime = true;
    static float cTime = 0.0;
    const float turnTime = 72.0f;
    const float angTurnTimeFact = 2.0f * M_PI / turnTime;

    if(autoTime) {
        cTime = cTime + deltaT;
        cTime = (cTime > turnTime) ? (cTime - turnTime) : cTime;
    }
    static float tTime = 0.0;
    const float TturnTime = 60.0f;
    const float TangTurnTimeFact = 1.0f / TturnTime;

    if(autoTime) {
        tTime = tTime + deltaT;
        tTime = (tTime > TturnTime) ? (tTime - TturnTime) : tTime;
    }

    const float ROT_SPEED = glm::radians(120.0f);
    const float MOVE_SPEED = 2.0f;
    static float ShowCloud = 1.0f;
    static float ShowTexture = 1.0f;
    static float subpassTimer = 0.0;

    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    // Standard procedure to quit when the ESC key is pressed
    if(glfwGetKey(window, GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    /*   ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
       ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 30.0f);
       ubo.proj[1][1] *= -1;
*/

    // The Fly model update proc.
    ubo.view = glm::rotate(glm::mat4(1), ROT_SPEED * r.x * deltaT,
                           glm::vec3(1, 0, 0)) * ubo.view;
    ubo.view = glm::rotate(glm::mat4(1), ROT_SPEED * r.y * deltaT,
                           glm::vec3(0, 1, 0)) * ubo.view;
    ubo.view = glm::rotate(glm::mat4(1), -ROT_SPEED * r.z * deltaT,
                           glm::vec3(0, 0, 1)) * ubo.view;
    ubo.view = glm::translate(glm::mat4(1), -glm::vec3(
            MOVE_SPEED * m.x * deltaT, /*MOVE_SPEED * m.y * deltaT*/0.0f, MOVE_SPEED * m.z * deltaT))
               * ubo.view;

    LightsBufferObject lbo{};
    lbo.lightPos = glm::vec3(0.0f, 0.2f, 0.0f);
    lbo.viewPos = glm::vec3(0.0f, 0.0f, 0.0f);
    lbo.lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    lbo.ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);
    lbo.diffuseColor = glm::vec3(0.5f, 0.5f, 0.5f);
    lbo.specularColor = glm::vec3(1.0f, 1.0f, 1.0f);
    lbo.shininess = 32.0f;

    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    memcpy(lightsBuffersMapped[currentImage], &lbo, sizeof(lbo));
}

void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, VkRenderPass &renderPass,
                         std::vector<VkFramebuffer> &swapChainFramebuffers, VkExtent2D &swapChainExtent,
                         VkPipeline &graphicsPipeline, VkBuffer &vertexBuffer, VkBuffer &indexBuffer,
                         VkPipelineLayout &pipelineLayout, std::vector<uint32_t> &indices,
                         std::vector<VkDescriptorSet> &descriptorSets, uint32_t &currentFrame) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swapChainExtent.width;
    viewport.height = (float) swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void createCommandBuffers(VkDevice &device, std::vector<VkCommandBuffer> &commandBuffers, VkCommandPool &commandPool,
                          const int maxFramesInFlight) {
    commandBuffers.resize(maxFramesInFlight);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
                  VkDeviceMemory& bufferMemory, VkDevice &device, VkPhysicalDevice &physicalDevice) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties, physicalDevice);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

VkCommandBuffer beginSingleTimeCommands(VkCommandPool &commandPool, VkDevice &device) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkDevice &device,
                           VkCommandPool &commandPool, VkQueue graphicsQueue) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandPool &commandPool,
                VkDevice &device, VkQueue &graphicsQueue) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(commandPool, device);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer, device, commandPool, graphicsQueue);
}

void createMatrixUniformBuffer(VkDevice &device, VkPhysicalDevice &physicalDevice, VkExtent2D &swapChainExtent,
                          std::vector<VkBuffer> &uniformBuffers, std::vector<VkDeviceMemory> &uniformBuffersMemory,
                          std::vector<void*> &uniformBuffersMapped,
                          const int maxFramesInFlight) {

    MatrixBufferObject mubo {};
    mubo.model[0] = glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f));
    mubo.model[1] = glm::translate(glm::mat4(1.0f), glm::vec3(15, 0, 0));
    mubo.model[2] = glm::translate(glm::mat4(1.0f), glm::vec3(15, 0, 0));
    mubo.model[3] = glm::translate(glm::mat4(1.0f), glm::vec3(15, 0, 0));

    VkDeviceSize bufferSize1 = sizeof(MatrixBufferObject);

    uniformBuffers.resize(maxFramesInFlight);
    uniformBuffersMemory.resize(maxFramesInFlight);
    uniformBuffersMapped.resize(maxFramesInFlight);

    for (size_t i = 0; i < maxFramesInFlight; i++) {
        createBuffer(bufferSize1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     uniformBuffers[i], uniformBuffersMemory[i], device, physicalDevice);
        vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize1, 0, &uniformBuffersMapped[i]);
    }
    std::cout << "Buffer Matrix Created \n";
}


void createUniformBuffers(VkDevice &device, VkPhysicalDevice &physicalDevice, VkExtent2D &swapChainExtent,
                          std::vector<VkBuffer> &uniformBuffers, std::vector<VkDeviceMemory> &uniformBuffersMemory,
                          std::vector<void*> &uniformBuffersMapped, std::vector<VkBuffer> &lightsBuffers,
                          std::vector<VkDeviceMemory> &lightsBuffersMemory, std::vector<void*> &lightsBuffersMapped,
                          const int maxFramesInFlight) {
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    float scaleFactor = 0.01f; // Change this value to scale down

    // Model matrix with scaling
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scaleFactor));

    ubo.model = scaleMatrix * ubo.model;
    ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 30.0f);
    ubo.proj[1][1] *= -1;
    ubo.view = glm::lookAt(glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    VkDeviceSize bufferSize1 = sizeof(UniformBufferObject);
    VkDeviceSize bufferSize2 = sizeof(LightsBufferObject);

    uniformBuffers.resize(maxFramesInFlight);
    uniformBuffersMemory.resize(maxFramesInFlight);
    uniformBuffersMapped.resize(maxFramesInFlight);

    lightsBuffers.resize(maxFramesInFlight);
    lightsBuffersMemory.resize(maxFramesInFlight);
    lightsBuffersMapped.resize(maxFramesInFlight);

    for (size_t i = 0; i < maxFramesInFlight; i++) {
        createBuffer(bufferSize1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     uniformBuffers[i], uniformBuffersMemory[i], device, physicalDevice);
        createBuffer(bufferSize2, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     lightsBuffers[i], lightsBuffersMemory[i], device, physicalDevice);

        vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize1, 0, &uniformBuffersMapped[i]);
        vkMapMemory(device, lightsBuffersMemory[i], 0, bufferSize2, 0, &lightsBuffersMapped[i]);
    }
}

void createIndexBuffer(VkDevice &device, VkPhysicalDevice &physicalDevice, std::vector<uint32_t> &indices,
                       VkCommandPool &commandPool, VkQueue &graphicsQueue, VkBuffer &indexBuffer,
                       VkDeviceMemory &indexBufferMemory) {
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size() * 1000;
    VkDeviceSize actualBufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory, device, physicalDevice);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t) actualBufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory, device, physicalDevice);

    copyBuffer(stagingBuffer, indexBuffer, bufferSize,
               commandPool, device, graphicsQueue);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height,
                       VkCommandPool &commandPool, VkDevice &device, VkQueue &graphicsQueue) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(commandPool, device);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
            width,
            height,
            1
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(commandBuffer, device, commandPool, graphicsQueue);
}