#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
/*
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
*/
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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
#include <datetimeapi.h>


#include "ObjectLoader.h"
#include "texturesManagement.h"
#include "CharInfoStructure.h"

const uint32_t WIDTH = 1920;
const uint32_t HEIGHT = 1080;

const std::string TEXTURE_PATH = "textures/";

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

//UniformBufferObject ubo{};

struct Scene{
    uint16_t index;
    std::vector<VkPipeline*> pipelineReferences;
    std::vector<VkPipelineLayout*> pipelineLayoutReferences;
    std::vector<VkDescriptorSetLayout*> descriptorSetLayoutsReferences;
    std::vector<std::vector<VkDescriptorSet>*> descriptorSetsReferences;

    Scene(){

    }

    Scene(
            uint16_t index,
            std::vector<VkPipeline*> pipelineReferences,
            std::vector<VkPipelineLayout*> pipelineLayoutReferences,
            std::vector<VkDescriptorSetLayout*> descriptorSetLayoutsReferences,
            std::vector<std::vector<VkDescriptorSet>*> descriptorSetsReferences
            ){
        this->index = index;
        this->pipelineReferences = std::move(pipelineReferences);
        this->pipelineLayoutReferences = std::move(pipelineLayoutReferences);
        this->descriptorSetLayoutsReferences = std::move(descriptorSetLayoutsReferences);
        this->descriptorSetsReferences = std::move(descriptorSetsReferences);
    }
};


VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};


class HelloTriangleApplication {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkCommandPool commandPool;
    VkCommandPool skyBoxCommandPool;

    VkImage colorImage;
    VkDeviceMemory colorImageMemory;
    VkImageView colorImageView;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    uint32_t mipLevels;
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;


    //SCENES
    Scene CurrentScene;
    Scene MenuScene;
    Scene ApplicationScene;


    //TEXTURES
    std::vector<VkImage> textureImages;
    std::vector<VkImageView> textureImageViews;
    std::vector<VkDeviceMemory> textureImageMemorys;
    std::vector<VkSampler> textureSamplers;


    //NORMALS
    std::vector<VkImage> normalImages;
    std::vector<VkImageView> normalImageViews;
    std::vector<VkDeviceMemory> normalImageMemorys;
    std::vector<VkSampler> normalSamplers;

    std::vector<ObjectInformation*> listObjectInfos;
    std::vector<ObjectInformation> listActualObjectInfos;
    bool isStart = true;

    std::vector<ObjectInformation> loadablesVector;

    ObjectLoader objectLoader;
    ObjectLoader skyBoxLoader;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    //Model buffer
    std::vector<VkBuffer> matrixUniformBuffers;
    std::vector<VkDeviceMemory> matrixUniformBuffersMemory;
    std::vector<void*> matrixUniformBuffersMapped;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    std::vector<VkBuffer> lightsBuffers;
    std::vector<VkDeviceMemory> lightsBuffersMemory;
    std::vector<void*> lightsBuffersMapped;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;

    bool framebufferResized = false;

    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

    bool keyPressed = false;
    bool keyPressedAdd = false;
    bool keyPressedAddSelect = false;
    bool keyPressedDelete = false;
    bool keyPressedLight = false;
    int currentTransformationModel = 0;
    uint32_t addObjectIndex = 0;
    bool mustAddObject = false;
    bool mustChangeSelectedObject = false;
    bool mustChangeScene = false;
    bool mustDelete = false;

    bool normalProj = true;

    double lastTime = glfwGetTime();

    std::vector<std::string> texturePaths;
    std::vector<std::string> normalPaths;


    //SkyBox PIPELINE
    VkDescriptorSetLayout DSLskyBox;
    VkPipelineLayout pipelineLayoutSkyBox;
    VkPipeline graphicsPipelineSkyBox;

    VkImage skyboxImage;
    VkDeviceMemory skyboxImageMemory;
    VkImageView skyboxImageView;
    VkSampler skyboxSampler;
    std::vector<VkDescriptorSet> descriptorSetsSkyBox;

    std::vector<ObjectInformation*> listSkyBoxInfos;
    std::vector<ObjectInformation> listActualSkyBoxInfos;

    std::vector<Vertex> tempVerticesSB;
    std::vector<skyBoxVertex> skyBoxVertices;
    std::vector<uint32_t> skyBoxIndices;

    VkBuffer vertexBufferSB;
    VkDeviceMemory vertexBufferMemorySB;
    VkBuffer indexBufferSB;
    VkDeviceMemory indexBufferMemorySB;

    std::string texturePathSB;

    //LIGHTS OBJECTS
    std::vector<glm::vec3> directionalLights;
    std::vector<glm::vec3> pointLights;


    //TEXT ELEMENTS
    VkDescriptorSetLayout DSLText;
    VkPipelineLayout pipelineLayoutText;
    VkPipeline graphicsPipelineText;

    VkImage TextImage[2];
    VkDeviceMemory TextImageMemory[2];
    VkImageView TextImageView[2];
    VkSampler TextSampler[2];
    std::vector<VkDescriptorSet> descriptorSetsText;

    //std::vector<textVertex> textVertices;
    //std::vector<uint32_t> textIndices;

    float sizeImage = 400;
    float heightImage = (float)sizeImage / HEIGHT;
    float widthImage = (float)sizeImage / WIDTH;
    std::vector<textVertex> textVertices = {
            {{-1, -1}, {1, 0}, 0},
            {{ -1 + widthImage, -1}, {0, 0}, 0},
            {{ -1 + widthImage,  -1 + heightImage}, {0, 1}, 0},
            {{-1,  -1 + heightImage}, {1, 1}, 0},
            {{-1 + widthImage, -1}, {1, 0}, 1},
            {{ -1 + 2 * widthImage, -1}, {0, 0}, 1},
            {{ -1 + 2 * widthImage,  -1 + heightImage}, {0, 1}, 1},
            {{-1 + widthImage,  -1 + heightImage}, {1, 1}, 1}
    };

    std::vector<uint32_t > textIndices = {0, 2, 1, 2, 0, 3,
                                          4, 6, 5, 6, 4, 7,
                                          };


    VkBuffer vertexBufferText;
    VkDeviceMemory vertexBufferMemoryText;
    VkBuffer indexBufferText;
    VkDeviceMemory indexBufferMemoryText;

    std::string texturePathText;






    //MENU SCENE
    VkDescriptorSetLayout DSLMenu;
    VkPipelineLayout pipelineLayoutMenu;
    VkPipeline graphicsPipelineMenu;

    VkImage menuImage;
    VkDeviceMemory menuImageMemory;
    VkImageView menuImageView;
    VkSampler menuSampler;
    std::vector<VkDescriptorSet> descriptorSetsMenu;

    std::vector<menuVertex> menuVertices = {
            {{-1, -1}, {1, 0}},
            {{ 1, -1}, {0, 0}},
            {{ 1,  1}, {0, 1}},
            {{-1,  1}, {1, 1}},
    };

    std::vector<uint32_t > menuIndices = {0, 2, 1, 2, 0, 3};

    VkBuffer vertexBufferMenu;
    VkDeviceMemory vertexBufferMemoryMenu;
    VkBuffer indexBufferMenu;
    VkDeviceMemory indexBufferMemoryMenu;

    std::string texturePathMenu;




    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    void initVulkan() {
        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
        createRenderPass();

        createCommandPool();
        createColorResources();
        createDepthResources();
        createFramebuffers();


        createScenes();

        createObjectVector();
        createSkyBoxVector();


        LoadSceneMenuInit();

        //CREATE THE FIRST PIPELINE DESCRIPTOR SET
        createDescriptorSetLayout();

        //CREATE THE SECOND PIPELINE DESCRIPTOR SET
        createSkyBoxDescriptorSetLayout();

        //CREATE TEXT PIPELINE DSL
        createTextDescriptorSetLayout();

        //CREATE MENU DSL
        createMenuDescriptorSetLayout();


        //CREATE THE FIRST GRAPHIC PIPELINE
        createGraphicsPipeline();

        //CREATE THE SECOND GRAPHICS PIPELINE
        createSkyBoxGraphicsPipeline();

        //CREATE TEXT GRAPHICS PIPELINE
        createTextGraphicsPipeline();

        //CREATE MENU GRAPHICS PIPELINE
        createMenuGraphicsPipeline();





        createDescriptorPool();

        LoadMenuDescriptors();

        createCommandBuffers(device, commandBuffers, commandPool, MAX_FRAMES_IN_FLIGHT);
        createSyncObjects();
    }

    /**
     * @brief The mainLoop function checks whether the application window is closed of not and if it is not
     * it keeps on iterating on a loop that calculates the last time of the last frame and the *deltaTime* that is the
     * difference between the *currentTime* (is also calculated) and the *lastTime*. Then, the loop executes the
     * functions for the user functionality (adding, removing models, transforming them...) and it properly updates the
     * uniform buffer and it executes glfwPollEvents to keep the application running until either an error occurs or
     * the window is closed. Then it checks the content of the boolean variable mustAddObject that is updated on the
     * function *addObject* based on the user input, if it is true then it executes *addObjectDynamic*. At the end of
     * the loop it executes *drawFrame* that renders a new frame. Before exiting the function we make use of
     * *vkDeviceWaitIdle(device)* since the drawFrame operations are asynchronous and cleaning up resources before
     * these operations finish is a bad idea so we will make it wait for the logicaal device to finish operations
     * before exiting mainLoop and destroying the window.

     */
    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            float currentTime = glfwGetTime();
            float deltaTime = currentTime - lastTime;
            lastTime = currentTime;

            closeEscape(window);

            if(CurrentScene.index == MenuScene.index){
                launchChangeScreen(window, mustChangeScene);
            }

            if(CurrentScene.index == ApplicationScene.index){
                //lights management
                addlight(keyPressedLight, window, pointLights, directionalLights);

                changeCurrentModel(keyPressed, window, currentTransformationModel, listObjectInfos);
                selectAddObjectIndex(keyPressedAddSelect, window, addObjectIndex, 0, loadablesVector.size()-1, mustChangeSelectedObject);
                addObject(keyPressedAdd, window, addObjectIndex, mustAddObject);
                deleteObject(keyPressedDelete, window, mustDelete);
                updateTransformationData(currentTransformationModel, window, listObjectInfos, deltaTime);
                //lookAtModel(currentTransformationModel, window, listObjectInfos, swapChainExtent);
                //changeIsometricView(window);
                changeOrthogonalView(window, WIDTH, HEIGHT, normalProj);
                changeIsometricView(window, WIDTH, HEIGHT, normalProj);
                regularProj(window, WIDTH, HEIGHT, normalProj);
                updateUniformBuffer(currentFrame, window, uniformBuffersMapped, lightsBuffersMapped, normalProj, pointLights, directionalLights);
            }

            glfwPollEvents();

            if(mustAddObject){
                addObjectDynamic();

                int a = 0;
            }

            if(mustDelete){
                deleteModel(currentTransformationModel);
                mustDelete = false;
                currentTransformationModel = 0;
            }

            if(mustChangeSelectedObject){
                changeSelectedObject();

                mustChangeSelectedObject = false;
            }

            if(mustChangeScene && CurrentScene.index == MenuScene.index){
                ChangeScene();

                mustChangeScene = false;
            }

            drawFrame();
        }

        vkDeviceWaitIdle(device);
    }

    void addObjectDynamic() {
        uint32_t verticesSize = sizeof(vertices[0]) * vertices.size();
        uint32_t indicesSize = sizeof(indices[0]) * indices.size();

        ObjectInformation objectInformation {};
        objectInformation.modelPath = "";//"furniture/Laptop/SAMSUNG_Laptop.obj";
        objectInformation.texturePath = "";//"furniture/Laptop/SLT_Dif.png";
        objectInformation.mustBeLoaded = true;
        objectInformation.modelMatrix = glm::mat4(1.0f);
        objectInformation.hasNormalMap = false;
        objectInformation.hasNormalMap = "furniture/Laptop/SLT_Dif.png";
        chooseObjectToAdd(addObjectIndex, objectInformation);

        listActualObjectInfos.push_back(objectInformation);
        objectLoader.addObject(&listActualObjectInfos[listActualObjectInfos.size() - 1], texturePaths, normalPaths, listObjectInfos, vertices, indices);

        for (int i = 0; i < listActualObjectInfos.size(); ++i) {
            listObjectInfos[i] = &listActualObjectInfos[i];
        }

        vkDeviceWaitIdle(device);

        vkDestroyBuffer(device, indexBuffer, nullptr);
        vkFreeMemory(device, indexBufferMemory, nullptr);

        vkDestroyBuffer(device, vertexBuffer, nullptr);
        vkFreeMemory(device, vertexBufferMemory, nullptr);

        //Recreate the new ones
        //TODO add a create functions with a size for the buffers
        createVertexBuffer(vertices, vertexBuffer, vertexBufferMemory);
        createIndexBuffer(device, physicalDevice, indices, commandPool, graphicsQueue,
                          indexBuffer, indexBufferMemory);


        //update the textureImages, ImageViews, ImageSamplers, ImageMemories
        //update the texture images and memories
        updateTextureImagesAdd(mipLevels, device, physicalDevice, commandPool, graphicsQueue, textureImages,
                               textureImageMemorys, listActualObjectInfos[listActualObjectInfos.size() - 1].texturePath);
        updateTextureImagesAdd(mipLevels, device, physicalDevice, commandPool, graphicsQueue, normalImages,
                               normalImageMemorys, listActualObjectInfos[listActualObjectInfos.size() - 1].normalPath);

        //update the texture image views
        updateTextureImageViewsAdd(device, textureImages.at(textureImages.size() - 1), mipLevels, textureImageViews);
        //update the texture image samplers
        updateTextureImageViewsAdd(device, normalImages.at(normalImages.size() - 1), mipLevels, normalImageViews);
        //update the texture image samplers
        updateTextureImageSamplersAdd(physicalDevice, device, textureSamplers);
        updateTextureImageSamplersAdd(physicalDevice, device, normalSamplers);
        //update the descriptors
        recreateDescriptorsAndPipeline();

        mustAddObject = false;
    }

    void createTextVertices(const std::string& text, std::unordered_map<uint32_t, CharInfo>& charMap, float scale, float startX, float startY) {

        float x = startX;
        float y = startY;

        for (char c : text) {
            if (charMap.count(c) == 0) continue;
            CharInfo& ci = charMap[c];

            float x0 = x + ci.xoffset * scale;
            float y0 = y - ci.yoffset * scale;
            float x1 = x0 + ci.width * scale;
            float y1 = y0 - ci.height * scale;

            float u0 = ci.x / 512.0f;
            float v0 = ci.y / 512.0f;
            float u1 = (ci.x + ci.width) / 512.0f;
            float v1 = (ci.y + ci.height) / 512.0f;

            textVertices.push_back({{x0, y0}, {u0, v0}});
            textVertices.push_back({{x1, y0}, {u1, v0}});
            textVertices.push_back({{x0, y1}, {u0, v1}});

            textVertices.push_back({{x1, y0}, {u1, v0}});
            textVertices.push_back({{x1, y1}, {u1, v1}});
            textVertices.push_back({{x0, y1}, {u0, v1}});

            textIndices.push_back(vertices.size()-1);
            textIndices.push_back(vertices.size()-2);
            textIndices.push_back(vertices.size()-3);

            textIndices.push_back(vertices.size()-4);
            textIndices.push_back(vertices.size()-5);
            textIndices.push_back(vertices.size()-6);

            x += ci.xadvance * scale;
        }
    }


    void chooseObjectToAdd(uint32_t objectChosen, ObjectInformation & objectInformation){

        objectInformation = loadablesVector.at(objectChosen);

        /*
        if(objectChosen == 0){
            ObjectInformation skyBoxObj {};
            objectInformation.modelPath = "skyBox/skyBox.obj";
            objectInformation.texturePath = "skybox/Daylight Box UV.png";
            objectInformation.mustBeLoaded = false;
            //objectInformation.modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(150.0f, 150.0f, 150.0f));
            objectInformation.modelMatrix = glm::mat4(1.0f);
            objectInformation.hasNormalMap = false;
            objectInformation.normalPath = "furniture/Laptop/SLT_Dif.png";

        }else if(objectChosen == 1){
            objectInformation.modelPath = "furniture/CoconutTree/coconutTree.obj";
            objectInformation.texturePath = "furniture/CoconutTree/coconutTreeTexture.jpg";
            objectInformation.mustBeLoaded = true;
            objectInformation.modelMatrix = glm::mat4(1.0f);
            objectInformation.hasNormalMap = false;
            objectInformation.normalPath = "furniture/Laptop/SLT_Dif.png";
        }else if(objectChosen == 2){
            objectInformation.modelPath = "furniture/Laptop/SAMSUNG_Laptop.obj";
            objectInformation.texturePath = "furniture/Laptop/SLT_Dif.png";
            objectInformation.mustBeLoaded = true;
            objectInformation.modelMatrix = glm::mat4(1.0f);
            objectInformation.hasNormalMap = false;
            objectInformation.normalPath = "furniture/Laptop/SLT_Dif.png";
        }*/
    }

    void recreateDescriptorsAndPipeline() {
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        vkFreeDescriptorSets(device, descriptorPool, descriptorSets.size(), descriptorSets.data());
        vkDestroyPipeline(device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        descriptorSets.clear();
        createDescriptorSetLayout();

        createGraphicsPipeline();

        createDescriptorSets();
    }

    void ChangeScene(){
        CurrentScene = ApplicationScene;

        LoadSceneApplication();
    }

    void changeSelectedObject(){
        ObjectInformation objectInformation = loadablesVector.at(addObjectIndex);


        vkDeviceWaitIdle(device);

        //FREE THE IMAGES AND EVERYTHING
        vkDestroyImageView(device, TextImageView[0], nullptr);
        vkFreeMemory(device, TextImageMemory[0], nullptr);
        vkDestroyImage(device, TextImage[0], nullptr);
        vkDestroySampler(device, TextSampler[0], nullptr);




        //LOAD THE IMAGES, IMAGE VIEWS, SAMPLER ETC ETC
        createTextureImage(mipLevels, device, physicalDevice, commandPool, graphicsQueue, TextImage[0], TextImageMemory[0], objectInformation.presentationPath);
        createTextureImageView(device, TextImage[0], mipLevels, TextImageView[0]);
        createTextureSampler(physicalDevice, device, TextSampler[0]);



        //REBUILD THE DESCRIPTOR
        updateTextDescriptorSets();

    }

    /*
     * @param The function will receive as a parameter the position of the model that needs to be erased
     * from listObjectInfos
     */
    void deleteModel(int posModel){
        ObjectInformation objectInformation {};
        objectInformation = listActualObjectInfos.at(posModel);

        /* 1: Delete position of the VertexBuffer and move the following positions one step to the lef so that there are
         no empty spots (same for the IndexBuffer)*/
        //Move the points in the vectors
        size_t sizeVertices = objectInformation.vertices.size();
        size_t sizeIndices = objectInformation.localIndices.size();

        size_t sizeUntilVertices = 0;
        size_t sizeUntilIndices = 0;

        for (int i = 0; i < posModel; ++i) {
            sizeUntilVertices += listActualObjectInfos[i].vertices.size();
            sizeUntilIndices += listActualObjectInfos[i].localIndices.size();
        }

        movePointsVertexVector(sizeUntilVertices, sizeVertices);
        movePointsIndicesVector(sizeUntilIndices, sizeIndices, sizeVertices);
        //Update the buffers


        vkQueueWaitIdle(graphicsQueue);

        //Destroy the previous ones
        vkDestroyBuffer(device, indexBuffer, nullptr);
        vkFreeMemory(device, indexBufferMemory, nullptr);

        vkDestroyBuffer(device, vertexBuffer, nullptr);
        vkFreeMemory(device, vertexBufferMemory, nullptr);

        //Recreate the new ones
        //TODO add a create functions with a size for the buffers
        createVertexBuffer(vertices, vertexBuffer, vertexBufferMemory);
        createIndexBuffer(device, physicalDevice, indices, commandPool, graphicsQueue,
                          indexBuffer, indexBufferMemory);




        // 2: Erase, remove or delete all the things related to the textures
        //TEXTURE IMAGE
        //TEXTURE MEMORY
        deleteTextureImages(device, textureImages, textureImageMemorys, posModel);
        deleteTextureImages(device, normalImages, normalImageMemorys, posModel);
        //TEXTURE IMAGE VIEW
        deleteTextureImagesViews(device, posModel, mipLevels, textureImageViews);
        deleteTextureImagesViews(device, posModel, mipLevels, normalImageViews);
        //TEXTURE SAMPLER
        deleteTextureImageSampler(device, textureSamplers, posModel);
        deleteTextureImageSampler(device, normalSamplers, posModel);

        texturePaths.erase(texturePaths.begin() + posModel);
        normalPaths.erase(normalPaths.begin() + posModel);

        // 3: Remove Matrix  and move all the following matrix one step to the left

        //SHOULD BE DONE BY DEFAULT

        // 4: Remove  object from listObjectInfos listActualObjectInfos

        listActualObjectInfos.erase(listActualObjectInfos.begin() + posModel);
        listObjectInfos.pop_back();

        //ALSO MOVE THE POINTS
        movePointsObjectInfos(posModel);

        // 5: Erase, delete, remove pipeline, descriptor...

        recreateDescriptorsAndPipeline();

        if(currentTransformationModel >= listObjectInfos.size()){
            currentTransformationModel = listObjectInfos.size() - 1;
        }
    }

    void movePointsVertexVector(uint32_t objectPos, uint32_t objectSizeVertices){
        vertices.erase(vertices.begin() + objectPos, vertices.begin() + objectPos + objectSizeVertices);

        auto displacement = [](Vertex x) { return Vertex{x.pos, x.color, x.texCoord, x.normal, x.objectIndex - 1}; };

        std::transform(vertices.begin() + objectPos, vertices.end(), vertices.begin() + objectPos, displacement);
    }

    void movePointsIndicesVector(uint32_t objectPos, uint32_t objectSizeIndices, uint32_t sizeVertices){
        indices.erase(indices.begin() + objectPos, indices.begin() + objectPos + objectSizeIndices);

        auto displacement = [sizeVertices](int x) { return x - sizeVertices; };

        std::transform(indices.begin() + objectPos, indices.end(), indices.begin() + objectPos, displacement);
    }

    void movePointsObjectInfos(uint32_t objectPos){
        auto displacement = [](Vertex x) { return Vertex{x.pos, x.color, x.texCoord, x.normal, x.objectIndex - 1}; };

        for (int i = objectPos; i < listObjectInfos.size(); ++i) {
            std::transform(listObjectInfos[i]->vertices.begin(), listObjectInfos[i]->vertices.end(), listObjectInfos[i]->vertices.begin(), displacement);
        }
    }

    void cleanupSwapChain() {
        vkDestroyImageView(device, colorImageView, nullptr);
        vkDestroyImage(device, colorImage, nullptr);
        vkFreeMemory(device, colorImageMemory, nullptr);

        vkDestroyImageView(device, depthImageView, nullptr);
        vkDestroyImage(device, depthImage, nullptr);
        vkFreeMemory(device, depthImageMemory, nullptr);

        for (auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }

        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(device, imageView, nullptr);
        }

        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }

    void cleanup() {
        cleanupSwapChain();

        vkDestroyPipeline(device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

        vkDestroyPipeline(device, graphicsPipelineSkyBox, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayoutSkyBox, nullptr);

        vkDestroyPipeline(device, graphicsPipelineText, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayoutText, nullptr);

        vkDestroyPipeline(device, graphicsPipelineMenu, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayoutMenu, nullptr);


        vkDestroyRenderPass(device, renderPass, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroyBuffer(device, uniformBuffers[i], nullptr);
            vkFreeMemory(device, uniformBuffersMemory[i], nullptr);

            vkDestroyBuffer(device, lightsBuffers[i], nullptr);
            vkFreeMemory(device, lightsBuffersMemory[i], nullptr);

            vkDestroyBuffer(device, matrixUniformBuffers[i], nullptr);
            vkFreeMemory(device, matrixUniformBuffersMemory[i], nullptr);
        }

        vkDestroyDescriptorPool(device, descriptorPool, nullptr);

        for (int i = 0; i < textureImages.size(); ++i) {
            vkDestroySampler(device, textureSamplers[i], nullptr);
            vkDestroyImageView(device, textureImageViews[i], nullptr);

            vkDestroyImage(device, textureImages[i], nullptr);
            vkFreeMemory(device, textureImageMemorys[i], nullptr);

            vkDestroySampler(device, normalSamplers[i], nullptr);
            vkDestroyImageView(device, normalImageViews[i], nullptr);

            vkDestroyImage(device, normalImages[i], nullptr);
            vkFreeMemory(device, normalImageMemorys[i], nullptr);
        }

        vkDestroySampler(device, textureSampler, nullptr);
        vkDestroyImageView(device, textureImageView, nullptr);

        vkDestroyImage(device, textureImage, nullptr);
        vkFreeMemory(device, textureImageMemory, nullptr);

        vkDestroySampler(device, skyboxSampler, nullptr);
        vkDestroyImageView(device, skyboxImageView, nullptr);

        vkDestroyImage(device, skyboxImage, nullptr);
        vkFreeMemory(device, skyboxImageMemory, nullptr);

        vkDestroySampler(device, TextSampler[0], nullptr);
        vkDestroyImageView(device, TextImageView[0], nullptr);

        vkDestroyImage(device, TextImage[0], nullptr);
        vkFreeMemory(device, TextImageMemory[0], nullptr);

        vkDestroySampler(device, TextSampler[1], nullptr);
        vkDestroyImageView(device, TextImageView[1], nullptr);

        vkDestroyImage(device, TextImage[1], nullptr);
        vkFreeMemory(device, TextImageMemory[1], nullptr);

        //MENU
        vkDestroySampler(device, menuSampler, nullptr);
        vkDestroyImageView(device, menuImageView, nullptr);

        vkDestroyImage(device, menuImage, nullptr);
        vkFreeMemory(device, menuImageMemory, nullptr);


        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, DSLskyBox, nullptr);
        vkDestroyDescriptorSetLayout(device, DSLText, nullptr);
        vkDestroyDescriptorSetLayout(device, DSLMenu, nullptr);

        vkDestroyBuffer(device, indexBuffer, nullptr);
        vkFreeMemory(device, indexBufferMemory, nullptr);

        vkDestroyBuffer(device, indexBufferSB, nullptr);
        vkFreeMemory(device, indexBufferMemorySB, nullptr);

        vkDestroyBuffer(device, indexBufferText, nullptr);
        vkFreeMemory(device, indexBufferMemoryText, nullptr);

        vkDestroyBuffer(device, indexBufferMenu, nullptr);
        vkFreeMemory(device, indexBufferMemoryMenu, nullptr);


        vkDestroyBuffer(device, vertexBuffer, nullptr);
        vkFreeMemory(device, vertexBufferMemory, nullptr);

        vkDestroyBuffer(device, vertexBufferSB, nullptr);
        vkFreeMemory(device, vertexBufferMemorySB, nullptr);

        vkDestroyBuffer(device, vertexBufferText, nullptr);
        vkFreeMemory(device, vertexBufferMemoryText, nullptr);

        vkDestroyBuffer(device, vertexBufferMenu, nullptr);
        vkFreeMemory(device, vertexBufferMemoryMenu, nullptr);


        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(device, commandPool, nullptr);

        vkDestroyDevice(device, nullptr);

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }

    void recreateSwapChain() {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(device);

        cleanupSwapChain();

        createSwapChain();
        createImageViews();
        createColorResources();
        createDepthResources();
        createFramebuffers();
    }

    void createInstance() {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;

            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    void setupDebugMessenger() {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    void createSurface() {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                physicalDevice = device;
                msaaSamples = getMaxUsableSampleCount();
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    VkSampleCountFlagBits getMaxUsableSampleCount() {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

        VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
        if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
        if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
        if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
        if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
        if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
        if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

        return VK_SAMPLE_COUNT_1_BIT;
    }

    void createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.sampleRateShading = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    }

    void createSwapChain() {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }

    void createImageViews() {
        swapChainImageViews.resize(swapChainImages.size());

        for (uint32_t i = 0; i < swapChainImages.size(); i++) {
            swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat,
                                                     VK_IMAGE_ASPECT_COLOR_BIT, 1, device);
        }
    }

    void createScenes(){

        MenuScene = Scene(
                0,
                {&graphicsPipelineMenu},
                {&pipelineLayoutMenu},
                {&DSLMenu},
                {&descriptorSetsMenu}
                );

        ApplicationScene = Scene(
                1,
                {&graphicsPipeline, &graphicsPipelineSkyBox, &graphicsPipelineText},
                {&pipelineLayout, &pipelineLayoutSkyBox, &pipelineLayoutText},
                {&descriptorSetLayout, &DSLskyBox, &DSLText},
                {&descriptorSets, &descriptorSetsSkyBox, &descriptorSetsMenu}
                );
        CurrentScene = MenuScene;
    }

    void LoadSceneMenu(){
        LoadSceneMenuInit();
        LoadMenuDescriptors();
    }

    void LoadSceneMenuInit(){
        createTextureImage(mipLevels, device, physicalDevice, commandPool, graphicsQueue, menuImage,
                           menuImageMemory, "presentations/animals/cat/catPresentation.png");
        createTextureImageView(device, menuImage, mipLevels, menuImageView);
        createTextureSampler(physicalDevice, device, menuSampler);


        createMenuVertexBuffer(menuVertices, vertexBufferMenu, vertexBufferMemoryMenu);
        createIndexBuffer(device, physicalDevice, menuIndices, commandPool, graphicsQueue,
                          indexBufferMenu, indexBufferMemoryMenu);
    }

    void LoadMenuDescriptors(){
        createMenuDescriptorSets();
    }

    void LoadSceneApplication(){
        LoadSceneApplicationInit();
        LoadApplicationDescriptors();
    }

    void LoadApplicationDescriptors(){
        //CREATE DESCRIPTOR SETS
        createDescriptorSets();

        //CREATE DESCRIPTOR SETS SKYBOX
        createSkyBoxDescriptorSets();

        //CREATE TEXT DESCRIPTOR SETS
        createTextDescriptorSets();
    }

    void LoadSceneApplicationInit(){
        //CREATE TEXTURES AND NORMALS
        createTextureImage(mipLevels, device, physicalDevice, commandPool, graphicsQueue, textureImage,
                           textureImageMemory, "fonts/tata_0.png");
        createTextureImages(mipLevels, device, physicalDevice, commandPool, graphicsQueue, textureImages,
                            textureImageMemorys, texturePaths);
        createTextureImages(mipLevels, device, physicalDevice, commandPool, graphicsQueue, normalImages,
                            normalImageMemorys, normalPaths);

        createTextureImage(mipLevels, device, physicalDevice, commandPool, graphicsQueue, TextImage[0],
                           TextImageMemory[0], "fonts/tata_0.png");
        createTextureImage(mipLevels, device, physicalDevice, commandPool, graphicsQueue, TextImage[1],
                           TextImageMemory[1], "fonts/tata_0.png");

        //TEXT IMAGE

        createTextureImageView(device, textureImage, mipLevels, textureImageView);
        createTextureImageViews(device, textureImages, mipLevels, textureImageViews);
        createTextureImageViews(device, normalImages, mipLevels, normalImageViews);
        createTextureImageView(device, TextImage[0], mipLevels, TextImageView[0]);
        createTextureImageView(device, TextImage[1], mipLevels, TextImageView[1]);


        createTextureSampler(physicalDevice, device, textureSampler);
        createTextureSamplers(physicalDevice, device, textureSamplers, texturePaths.size());
        createTextureSamplers(physicalDevice, device, normalSamplers, normalPaths.size());
        createTextureSampler(physicalDevice, device, TextSampler[0]);
        createTextureSampler(physicalDevice, device, TextSampler[1]);

        //CREATE SKYBOX
        createSkyBoxTextures(texturePathSB);

        createObjectLoader();
        createSkyBoxLoader();


        launchObjectLoader();
        launchSkyBoxLoader();

        createVertexBuffer(vertices, vertexBuffer, vertexBufferMemory);

        //Create SB VBuffer
        createSBVertexBuffer(skyBoxVertices, vertexBufferSB, vertexBufferMemorySB);

        //CREATE TEXT VBuffer
        createTextVertexBuffer(textVertices, vertexBufferText, vertexBufferMemoryText);

        //Create first IBuffer
        createIndexBuffer(device, physicalDevice, indices, commandPool, graphicsQueue,
                          indexBuffer, indexBufferMemory);
        //Create SB IBuffer
        createIndexBuffer(device, physicalDevice, skyBoxIndices, commandPool, graphicsQueue,
                          indexBufferSB, indexBufferMemorySB);
        //CREATE TEXT IBuffer
        createIndexBuffer(device, physicalDevice, textIndices, commandPool, graphicsQueue,
                          indexBufferText, indexBufferMemoryText);

        createUniformBuffers(device, physicalDevice, swapChainExtent, uniformBuffers, uniformBuffersMemory,
                             uniformBuffersMapped, lightsBuffers, lightsBuffersMemory, lightsBuffersMapped,
                             MAX_FRAMES_IN_FLIGHT);
        createMatrixUniformBuffer(device, physicalDevice, swapChainExtent, matrixUniformBuffers, matrixUniformBuffersMemory,
                                  matrixUniformBuffersMapped, MAX_FRAMES_IN_FLIGHT);
    }

    void UnLoadSceneApplication(){

    }

    void createRenderPass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = msaaSamples;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = findDepthFormat();
        depthAttachment.samples = msaaSamples;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription colorAttachmentResolve{};
        colorAttachmentResolve.format = swapChainImageFormat;
        colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentResolveRef{};
        colorAttachmentResolveRef.attachment = 2;
        colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
        subpass.pResolveAttachments = &colorAttachmentResolveRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void createMenuDescriptorSetLayout(){
        VkDescriptorSetLayoutBinding menuLayoutBinding{};
        menuLayoutBinding.binding = 0;
        menuLayoutBinding.descriptorCount = 1;
        menuLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        menuLayoutBinding.pImmutableSamplers = nullptr;
        menuLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::array<VkDescriptorSetLayoutBinding, 1> bindings = {menuLayoutBinding};
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &DSLMenu) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout of the Text!");
        }

    }

    void createTextDescriptorSetLayout(){
        VkDescriptorSetLayoutBinding textLayoutBinding{};
        textLayoutBinding.binding = 0;
        textLayoutBinding.descriptorCount = 2;
        textLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        textLayoutBinding.pImmutableSamplers = nullptr;
        textLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::array<VkDescriptorSetLayoutBinding, 1> bindings = {textLayoutBinding};
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &DSLText) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout of the Text!");
        }

    }

    void createSkyBoxDescriptorSetLayout(){
        VkDescriptorSetLayoutBinding skyBoxLayoutBinding{};
        skyBoxLayoutBinding.binding = 0;
        skyBoxLayoutBinding.descriptorCount = 1;
        skyBoxLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        skyBoxLayoutBinding.pImmutableSamplers = nullptr;
        skyBoxLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutBinding matrixLayoutBinding{};
        matrixLayoutBinding.binding = 1;
        matrixLayoutBinding.descriptorCount = 1;
        matrixLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        matrixLayoutBinding.pImmutableSamplers = nullptr;
        matrixLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = {skyBoxLayoutBinding, matrixLayoutBinding};
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &DSLskyBox) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout of the skyBox!");
        }
    }

    void createDescriptorSetLayout() {
        std::vector<VkDescriptorSetLayoutBinding> bindings = {
                {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}, // UBO for vertex
                {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // Texture
                {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // UBO for lights
                {3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}, // UBO for matrix
                {4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(texturePaths.size()), VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // Multiple textures
                {5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(normalPaths.size()), VK_SHADER_STAGE_VERTEX_BIT, nullptr} // Multiple normals
        };

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    void createMenuGraphicsPipeline(){
        auto vertShaderCode = readFile("shaders/menuvert.spv");
        auto fragShaderCode = readFile("shaders/menufrag.spv");

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        auto bindingDescription = menuVertex::getBindingDescription();
        auto attributeDescriptions = menuVertex::getAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = msaaSamples;
        multisampling.sampleShadingEnable = VK_TRUE; // enable sample shading in the pipeline
        multisampling.minSampleShading = .2f; // min fraction for sample shading; closer to one is smoother

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::vector<VkDynamicState> dynamicStates = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &DSLMenu;

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayoutMenu) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayoutMenu;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipelineMenu) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        vkDestroyShaderModule(device, vertShaderModule, nullptr);

    }

    void createTextGraphicsPipeline(){
        auto vertShaderCode = readFile("shaders/textvert.spv");
        auto fragShaderCode = readFile("shaders/textfrag.spv");

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        auto bindingDescription = textVertex::getBindingDescription();
        auto attributeDescriptions = textVertex::getAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = msaaSamples;
        multisampling.sampleShadingEnable = VK_TRUE; // enable sample shading in the pipeline
        multisampling.minSampleShading = .2f; // min fraction for sample shading; closer to one is smoother

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::vector<VkDynamicState> dynamicStates = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &DSLText;

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayoutText) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayoutText;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipelineText) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        vkDestroyShaderModule(device, vertShaderModule, nullptr);
    }

    void createSkyBoxGraphicsPipeline(){
        auto vertShaderCode = readFile("shaders/skyBoxvert.spv");
        auto fragShaderCode = readFile("shaders/skyBoxfrag.spv");

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        auto bindingDescription = skyBoxVertex::getBindingDescription();
        auto attributeDescriptions = skyBoxVertex::getAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = msaaSamples;
        multisampling.sampleShadingEnable = VK_TRUE; // enable sample shading in the pipeline
        multisampling.minSampleShading = .2f; // min fraction for sample shading; closer to one is smoother

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::vector<VkDynamicState> dynamicStates = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &DSLskyBox;

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayoutSkyBox) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayoutSkyBox;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipelineSkyBox) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline for the Sky Box!");
        }

        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        vkDestroyShaderModule(device, vertShaderModule, nullptr);
    }

    void createGraphicsPipeline() {
        auto vertShaderCode = readFile("shaders/vert.spv");
        auto fragShaderCode = readFile("shaders/frag.spv");

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = msaaSamples;
        multisampling.sampleShadingEnable = VK_TRUE; // enable sample shading in the pipeline
        multisampling.minSampleShading = .2f; // min fraction for sample shading; closer to one is smoother

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::vector<VkDynamicState> dynamicStates = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        vkDestroyShaderModule(device, vertShaderModule, nullptr);
    }


    void createSkyBoxTextures(std::string texturePathSB){
        createTextureImage(mipLevels, device, physicalDevice, commandPool, graphicsQueue, skyboxImage,
                           skyboxImageMemory, "fonts/tata_0.png");
        createTextureImageView(device, skyboxImage, mipLevels, skyboxImageView);
        createTextureSampler(physicalDevice, device, skyboxSampler);
    }

    void createSkyBoxVector(){
        ObjectInformation skyBoxObj {};
        skyBoxObj.modelPath = "skyBox/skyBox.obj";
        skyBoxObj.texturePath = "skybox/Daylight Box UV.png";
        skyBoxObj.mustBeLoaded = true;
        skyBoxObj.modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(20.0f, 20.0f, 20.0f));

        listActualSkyBoxInfos.push_back(skyBoxObj);
        listSkyBoxInfos.push_back(&listActualSkyBoxInfos[0]);

        isStart = true;

        texturePathSB = "skybox/Daylight Box UV.png";
    }

    void createObjectVector(){
        ObjectInformation objCarpet = ObjectInformation(
                "furniture/Carpet/round-carpet.obj",
                glm::scale(glm::mat4(1.0f), glm::vec3(3.0f, 3.0f, 3.0f)),
                "furniture/Carpet/red_carpet.jpg",
                false);
        objCarpet.scaleModel(3.1f);
        objCarpet.translateModel(0.0f, 0.0f, -7.5f);

        ObjectInformation pokerRoom{};
        pokerRoom.modelPath = "furniture/Poker Room/scene.gltf";
        pokerRoom.texturePath = "furniture/House/mondrian.png";
        pokerRoom.mustBeLoaded = true;
        pokerRoom.modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(100.0f, 100.0f, 100.0f));
        pokerRoom.hasNormalMap = false;
        pokerRoom.normalPath = "";
        pokerRoom.isGltf = true;
      //  pokerRoom.modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));


        ObjectInformation objTurret {};
        objTurret.modelPath = "turret.obj";
        objTurret.texturePath = "furniture/House/mondrian.png";
        objTurret.mustBeLoaded = true;
        objTurret.modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(15.0f, 15.0f, 15.0f));
        objTurret.hasNormalMap = false;
        objTurret.isGltf = false;
        objTurret.normalPath = "";

/*
        ObjectInformation objHouse {};
        objHouse.modelPath = "furniture/House/house_04.obj";
        objHouse.texturePath = "furniture/House/mondrian.png";
        objHouse.mustBeLoaded = true;
        objHouse.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(60, 0, 0));
        objHouse.hasNormalMap = false;
        objHouse.isGltf = false;
        objHouse.normalPath = "";
*/

        ObjectInformation objGLTF {};
        objGLTF.modelPath = "furniture/Bed/scene.gltf";
        objGLTF.texturePath = "furniture/House/mondrian.png";
        objGLTF.mustBeLoaded = true;
        objGLTF.modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(60, 60, 60));
        objGLTF.hasNormalMap = false;
        objGLTF.isGltf = true;
        objGLTF.normalPath = "";

/*
        ObjectInformation objMorris {};
        objMorris.modelPath = "furniture/MorrisChair/morrisChair.obj";
        objMorris.texturePath = "furniture/MorrisChair/morrisChair_smallChairMat_BaseColor.tga.png";
        objMorris.mustBeLoaded = true;
        objMorris.modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(3.0f, 3.0f, 3.0f));
        objMorris.hasNormalMap = true;
        objMorris.normalPath = "furniture/MorrisChair/morrisChair_smallChairMat_Normal.tga.png";
*/

        ObjectInformation objMorris = ObjectInformation(
                "furniture/MorrisChair/morrisChair.obj",
                glm::scale(glm::mat4(1.0f), glm::vec3(3.0f, 3.0f, 3.0f)),
                "furniture/MorrisChair/morrisChair_smallChairMat_BaseColor.tga.png",
                false);
               // "furniture/MorrisChair/morrisChair_smallChairMat_BaseColor.tga.png"


        ObjectInformation objWildWestPiano = ObjectInformation(
                "furniture/WildWestPiano/Piano.obj",
                glm::scale(glm::mat4(1.0f), glm::vec3(200.0f, 200.0f, 200.0f)),
                "furniture/WildWestPiano/Piano_Wood_BaseColor.png",
                false
                );

        objWildWestPiano.translateModel(7.0f, 0.0f, -0.4f);
        objWildWestPiano.rotateModel(60.0f, 'z');

        ObjectInformation objTVCabinet = ObjectInformation(
                "furniture/TVCabinet/Tes.obj",
                glm::scale(glm::mat4(1.0f), glm::vec3(0.55f, 0.55f, 0.55f)),
                "furniture/TVCabinet/TCA_Dif.png",
                false
        );
        objTVCabinet.translateModel(-450.0f, 1600.0f, -180.0f);
        objTVCabinet.rotateModel(90.0f, 'x');

        ObjectInformation objTruck = ObjectInformation(
                "Cars/keytruck.obj",
                glm::scale(glm::mat4(1.0f), glm::vec3(100.0f, 100.0f, 100.0f)),
                "Cars/Mitsubishi.png",
                false
        );

        ObjectInformation objCat = ObjectInformation(
                "animals/cat/Cat.obj",
                glm::scale(glm::mat4(1.0f), glm::vec3(3.0f)),
                "animals/cat/Cat_diffuse.jpeg",
                false
        );

        objCat.translateModel(110.0f, 20.0f, -24.0);
        objCat.rotateModel(90.0f, 'x');
        objCat.rotateModel(15.0f, 'y');

        ObjectInformation floor = ObjectInformation(
                "furniture/CyberpunkFlooring/scene.gltf",
                glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f)),
                "furniture/CyberpunkFlooring/wood_livingroom.jpg",
                true
        );

        floor.translateModel(0.0f, 0.0f, -96.0f);

        ObjectInformation obj = ObjectInformation(
                "furniture/MorrisChair/morrisChair.obj",
                glm::scale(glm::mat4(1.0f), glm::vec3(3.0f, 3.0f, 3.0f)),
                "furniture/MorrisChair/morrisChair_smallChairMat_BaseColor.tga.png",
                false
        );

        ObjectInformation objPlane{};
        objPlane.modelPath = "furniture/MorrisChair/morrisChair.obj";
        objPlane.texturePath = "furniture/MorrisChair/morrisChair_smallChairMat_BaseColor.tga.png";
        objPlane.mustBeLoaded = false;
        objPlane.modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1000.0f, 1000.0f, 1000.0f));
        objPlane.hasNormalMap = true;
        objPlane.normalPath = "furniture/MorrisChair/morrisChair_smallChairMat_Normal.tga.png";
        objPlane.vertices = {
                Vertex{glm::vec3(1.0, 1.0, 0), glm::vec3(1.0, 1.0, 1.0), glm::vec2(1.0, 1.0), glm::vec3(0.0, 0.0, 1.0), 3, false, glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)},
                Vertex{glm::vec3(-1.0, 1.0, 0), glm::vec3(1.0, 1.0, 1.0), glm::vec2(1.0, 1.0), glm::vec3(0.0, 0.0, 1.0), 3, false, glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)},
                Vertex{glm::vec3(-1.0, -1.0, 0), glm::vec3(1.0, 1.0, 1.0), glm::vec2(1.0, 1.0), glm::vec3(0.0, 0.0, 1.0), 3, false, glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)},
                Vertex{glm::vec3(1.0, -1.0, 0), glm::vec3(1.0, 1.0, 1.0), glm::vec2(1.0, 1.0), glm::vec3(0.0, 0.0, 1.0), 3, false, glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)}
        };
        objPlane.localIndices = {
                0, 1, 3, 2, 3, 1
        };

        listActualObjectInfos.push_back(floor);
        listActualObjectInfos.push_back(objWildWestPiano);
        listActualObjectInfos.push_back(objTVCabinet);
        listActualObjectInfos.push_back(objCarpet);
        listActualObjectInfos.push_back(objCat);
        //listActualObjectInfos.push_back(objTruck);
       //listActualObjectInfos.push_back(pokerRoom);
        //listActualObjectInfos.push_back(objTurret);
       // listActualObjectInfos.push_back(objHouse);
        listActualObjectInfos.push_back(objMorris);
        listActualObjectInfos.push_back(objPlane);
        //listActualObjectInfos.push_back(objPlane);
        //listActualObjectInfos.push_back(objGLTF);

        listObjectInfos.push_back(&listActualObjectInfos[0]);
        listObjectInfos.push_back(&listActualObjectInfos[1]);
        listObjectInfos.push_back(&listActualObjectInfos[2]);
       listObjectInfos.push_back(&listActualObjectInfos[3]);
        listObjectInfos.push_back(&listActualObjectInfos[4]);
        listObjectInfos.push_back(&listActualObjectInfos[5]);
        listObjectInfos.push_back(&listActualObjectInfos[6]);

        isStart = true;

        for (int i = 0; i < listObjectInfos.size(); ++i) {
            texturePaths.push_back(listActualObjectInfos.at(i).texturePath);

            if(listActualObjectInfos.at(i).hasNormalMap){
                normalPaths.push_back(listActualObjectInfos.at(i).normalPath);
            }else{
                normalPaths.push_back("furniture/MorrisChair/morrisChair_smallChairMat_Normal.tga.png");
            }
        }
    }

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

    void createSkyBoxCommandPool(){
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

        VkCommandPoolCreateInfo skyBoxPoolInfo{};
        skyBoxPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        skyBoxPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        skyBoxPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(device, &skyBoxPoolInfo, nullptr, &skyBoxCommandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create sky box command pool!");
        }
    }

    void createCommandPool() {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics command pool!");
        }
    }

    void createDepthResources() {
        VkFormat depthFormat = findDepthFormat();

        createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples,
                    depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory, device, physicalDevice);
        depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1, device);
    }

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!");
    }

    VkFormat findDepthFormat() {
        return findSupportedFormat(
                {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    bool hasStencilComponent(VkFormat format) {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    /*
    void loadTextures(std::vector<tinyobj::material_t> materials){
        textureImages.resize(materials.size());
        textureImageViews.resize(materials.size());
        textureSamplers.resize(materials.size());

        //std::vector<std::string> = {};

        for (size_t i = 0; i < materials.size(); ++i) {
            const std::string& textureFile = materials[i].diffuse_texname;
            if (!textureFile.empty()) {
                // Load the texture using your preferred image loading library (e.g., stb_image)
                // Create Vulkan image, image view, and sampler for each texture
                //loadTexture(textureFile, textureImages[i], textureImageViews[i], textureSamplers[i]);
            }
        }
    }
*/

    void createColorResources() {
        VkFormat colorFormat = swapChainImageFormat;

        createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, colorFormat,
                    VK_IMAGE_TILING_OPTIMAL,VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory,  device, physicalDevice);
        colorImageView = createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, device);
    }

    /*void loadSceneElements(std::vector<std::array<float,6>> &vertices){

        vertices.clear();
        indices.clear();

        // Generate vertices
        for (int i = 0; i <= rings; ++i) {
            float theta = i * M_PI / rings;
            float sinTheta = sin(theta);
            float cosTheta = cos(theta);

            for (int j = 0; j < slices; ++j) {
                float phi = j * 2.0f * M_PI / slices;
                float sinPhi = sin(phi);
                float cosPhi = cos(phi);

                float x = radius * cosPhi * sinTheta;
                float y = radius * cosTheta;
                float z = radius * sinPhi * sinTheta;

                std::array<float, 6> vertex = {x, y, z, x / radius, y / radius, z / radius};
                vertices.push_back(vertex);
            }
        }

        // Generate indices
        for (int i = 0; i < rings; ++i) {
            for (int j = 0; j < slices; ++j) {
                int first = (i * slices) + j;
                int second = first + slices;

                indices.push_back(first);
                indices.push_back((second + 1) % (rings * slices));
                indices.push_back(second);

                indices.push_back(first);
                indices.push_back((first + 1) % (rings * slices));
                indices.push_back((second + 1) % (rings * slices));
            }
        }
    }*/

    /*
    void loadSceneCone(){
        int slices = 40;
        float height = 0.5f;
        float radius = 0.33f;

        vertices.resize(2*slices +1);
        indices.resize(3 * 2 * slices);

        vertices[2*slices] = Vertex{{0.0f, height/2, 0.0f}, {1.0, 1.0, 1.0}, {0.5, 1.0}, {0.0f, 1.0f, 0.0f}};
        //vertices[2*slices] = {0.0f, height/2, 0.0f, 0.0f, 1.0f, 0.0f};
        for(int i = 0; i < slices; i++) {
            float ang = 2*M_PI * (float)i / (float)slices;

            // BASE POINT
            vertices[i] = Vertex{{radius * cos(ang), -height/2.0f, radius * sin(ang)}, {1.0, 1.0, 1.0}, {0.5f + 0.5f * cos(ang), 0.5f + 0.5f * sin(ang)}, {0.0f, -1.0f, 0.0f}};
            //vertices[i] = {radius * cos(ang), -height/2.0f, radius * sin(ang), 0.0f, -1.0f, 0.0f};

            // SIDES BASE
            glm::vec3 sideNormal = glm::normalize(glm::vec3(radius * cos(ang), radius, radius * sin(ang)));

            vertices[slices + i] = Vertex{{radius * cos(ang), -height/2.0f, radius * sin(ang)}, {1.0, 1.0, 1.0}, {i / (float)slices, 0.0f}, {sideNormal.x, sideNormal.y, sideNormal.z}};
            //vertices[slices + i] = {radius * cos(ang), -height/2.0f, radius * sin(ang), sideNormal.x, sideNormal.y, sideNormal.z};

            // BOTTOM BASE
            indices[3*i  ] = 0;
            indices[3*i+1] = i;
            indices[3*i+2] = (i+1) % slices;

            // TRIANGLES
            indices[3*(i + slices)  ] = 2*slices;
            indices[3*(i + slices) + 1] = ((i+1) % slices) + slices;
            indices[3*(i + slices) + 2] = i + slices;
        }
    }

    void loadSceneSphereElements(){
        int amountverts = 40 * (40 - 1) + 2;
        int amountinds = 3 * (40 - 1) * (40 * 2);
        vertices.resize(amountverts);
        indices.resize(amountinds);

        vertices[amountverts - 2] = Vertex{{0.0f,-0.5,0.0f}, {1.0, 1.0, 1.0}, {0.0, 0.0}, {0.0f, -1.0f, 0.0f}};

        vertices[amountverts - 1] = Vertex{{0.0f,0.5,0.0f}, {1.0, 1.0, 1.0}, {0.0, 0.0}, {0.0f, 1.0f, 0.0f}};


        for(int i = 1; i < 40; i++) {
            float angi = M_PI * (float)i / (float)40;

            for(int j = 0; j < 40; j++) {
                int offset = (i-1) * 40;
                float angj = 2*M_PI * (float)j / (float)40;

                glm::vec3 norma = glm::normalize(glm::vec3(0.25* cos(angj) * sin(angi), 0.25* cos(angi), 0.25* sin(angj) * sin(angi)));
                vertices[j + offset] = Vertex{{0.25* cos(angj) * sin(angi), 0.25* cos(angi), 0.25* sin(angj) * sin(angi)}, {1.0, 1.0, 1.0}, {0.0, 0.0}, {0.25* cos(angj) * sin(angi), 0.25* cos(angi), 0.25* sin(angj) * sin(angi)}};


                if(j + offset + 40 <= amountverts - 2){
                    indices[2 * 3 * (j + offset)] = (j+1) % 40 + offset;
                    indices[2 * 3 * (j + offset) + 2] = j + offset;
                    indices[2 * 3 * (j + offset) + 1] = j + 40 + offset;

                    indices[2 * 3 * (j + offset) + 3] = j + 40 + offset;
                    indices[2 * 3 * (j + offset) + 5] = (j+1) % 40 + offset + 40;
                    indices[2 * 3 * (j + offset) + 4] = (j+1) % 40 + offset;
                }
            }
        }



        for(int j = 0; j < 40; j++) {
            //TOP CONE
            int offsetTop = (40-2) * (40 * 2);
            indices[3 * (j + offsetTop)] = amountverts - 1;
            indices[3 * (j + offsetTop) + 1] = (j+1) % 40;
            indices[3 * (j + offsetTop) + 2] = j;

            //BOTTOM CONE
            int offsetBottom = (40-2) * (40 * 2) + 10;
            indices[3 * (j + offsetBottom)] = amountverts - 2;
            indices[3 * (j + offsetBottom) + 1] = j + (40-2) * 10;
            indices[3 * (j + offsetBottom) + 2] = (j + 1) % 10 + (40-2) * 10;
        }
    }

    void loadModel() {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        std::string modelPath = "models/furniture/MorrisChair/morrisChair.obj";
        std::string modelPathWaterMelon = "models/furniture/Laptop/SAMSUNG_Laptop.obj";
        std::string modelPath1 = "models/viking_room.obj";

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPathWaterMelon.c_str())) {
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
            }
        }

    }

*/
    void createSkyBoxLoader(){
        skyBoxLoader = ObjectLoader(&listSkyBoxInfos, &tempVerticesSB, &skyBoxIndices);
    }

    void createObjectLoader(){
        objectLoader = ObjectLoader(&listObjectInfos, &vertices, &indices);
    }

    void launchSkyBoxLoader(){
        skyBoxLoader.loadAllElements();
        skyBoxLoader.fillVertexAndIndices();
        skyBoxLoader.transformVertex(tempVerticesSB, skyBoxVertices);
    }

    void launchObjectLoader(){
        objectLoader.createLoadablesVector(loadablesVector);
        objectLoader.loadAllElements();
        objectLoader.fillVertexAndIndices();
    }

    void updateIndexBuffer(const std::vector<uint32_t>& newIndices, uint32_t currentIndicesSize) {
        VkDeviceSize newSize = sizeof(newIndices[0]) * newIndices.size();
        VkDeviceSize currentSize = currentIndicesSize;
        VkDeviceSize totalSize = newSize + currentSize;

        if (true) {
            VkBuffer newBuffer;
            VkDeviceMemory newBufferMemory;

            // Calculate new buffer size, ensure it has sufficient space
            VkDeviceSize newBufferSize = totalSize * 2;

            // Create the new buffer with necessary usage flags
            createBuffer(newBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, newBuffer, newBufferMemory, device, physicalDevice);

            // Create a staging buffer for the new data
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            createBuffer(newSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         stagingBuffer, stagingBufferMemory, device, physicalDevice);

            // Map and copy the new data to the staging buffer
            void* data;
            vkMapMemory(device, stagingBufferMemory, 0, newSize, 0, &data);
            memcpy(data, newIndices.data(), static_cast<size_t>(newSize));
            vkUnmapMemory(device, stagingBufferMemory);

            // Copy the existing data to the new buffer
            VkCommandBuffer commandBuffer = beginSingleTimeCommands(commandPool, device);
            VkBufferCopy copyRegion{};
            copyRegion.srcOffset = 0;
            copyRegion.dstOffset = 0;
            copyRegion.size = currentSize;
            vkCmdCopyBuffer(commandBuffer, indexBuffer, newBuffer, 1, &copyRegion);

            // Copy the new data to the new buffer
            copyRegion.srcOffset = 0;
            copyRegion.dstOffset = currentSize; // Append the new data at the end of the existing data
            copyRegion.size = newSize;
            vkCmdCopyBuffer(commandBuffer, stagingBuffer, newBuffer, 1, &copyRegion);
            endSingleTimeCommands(commandBuffer, device, commandPool, graphicsQueue);

            // Clean up the old buffer
            vkDestroyBuffer(device, indexBuffer, nullptr);
            vkFreeMemory(device, indexBufferMemory, nullptr);

            // Replace the old buffer with the new buffer
            indexBuffer = newBuffer;
            indexBufferMemory = newBufferMemory;

            // Clean up the staging buffer
            vkDestroyBuffer(device, stagingBuffer, nullptr);
            vkFreeMemory(device, stagingBufferMemory, nullptr);

        } else {
            // No need to reallocate the buffer, just update with the new indices

            // Create a staging buffer for the new data
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            createBuffer(newSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         stagingBuffer, stagingBufferMemory, device, physicalDevice);

            // Map and copy the new data to the staging buffer
            void* data;
            vkMapMemory(device, stagingBufferMemory, 0, newSize, 0, &data);
            memcpy(data, newIndices.data(), static_cast<size_t>(newSize));
            vkUnmapMemory(device, stagingBufferMemory);

            // Copy the new data from the staging buffer to the index buffer
            VkCommandBuffer commandBuffer = beginSingleTimeCommands(commandPool, device);
            VkBufferCopy copyRegion{};
            copyRegion.srcOffset = 0;
            copyRegion.dstOffset = currentSize; // Append the new data at the end of the existing data
            copyRegion.size = newSize;
            vkCmdCopyBuffer(commandBuffer, stagingBuffer, indexBuffer, 1, &copyRegion);
            endSingleTimeCommands(commandBuffer, device, commandPool, graphicsQueue);

            // Clean up the staging buffer
            vkDestroyBuffer(device, stagingBuffer, nullptr);
            vkFreeMemory(device, stagingBufferMemory, nullptr);
        }
    }

    //TODO CHECK BUFFER SIZE
    void updateVertexBuffer(const std::vector<Vertex>& newVertices, uint32_t currentBuffSize) {
        VkDeviceSize newSize = sizeof(newVertices[0]) * newVertices.size();
        VkDeviceSize currentSize = currentBuffSize;
        VkDeviceSize totalSize = newSize + currentSize;

        if (true) {
            // New buffer size if the current buffer is too small
            VkBuffer newBuffer;
            VkDeviceMemory newBufferMemory;
            VkDeviceSize newBufferSize = totalSize * 2;

            createBuffer(newBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, newBuffer, newBufferMemory, device, physicalDevice);

            // Create a staging buffer for the new data
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            createBuffer(newSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         stagingBuffer, stagingBufferMemory, device, physicalDevice);

            // Map and copy the new data to the staging buffer
            void* data;
            vkMapMemory(device, stagingBufferMemory, 0, newSize, 0, &data);
            memcpy(data, newVertices.data(), static_cast<size_t>(newSize));
            vkUnmapMemory(device, stagingBufferMemory);

            // Copy the existing data to the new buffer
            VkCommandBuffer commandBuffer = beginSingleTimeCommands(commandPool, device);
            VkBufferCopy copyRegion{};
            copyRegion.srcOffset = 0;
            copyRegion.dstOffset = 0;
            copyRegion.size = currentSize;
            vkCmdCopyBuffer(commandBuffer, vertexBuffer, newBuffer, 1, &copyRegion);

            // Copy the new data to the new buffer
            copyRegion.srcOffset = 0;
            copyRegion.dstOffset = currentSize; // Append the new data at the end of the existing data
            copyRegion.size = newSize;
            vkCmdCopyBuffer(commandBuffer, stagingBuffer, newBuffer, 1, &copyRegion);
            endSingleTimeCommands(commandBuffer, device, commandPool, graphicsQueue);

            // Clean up the old buffer
            vkDestroyBuffer(device, vertexBuffer, nullptr);
            vkFreeMemory(device, vertexBufferMemory, nullptr);

            // Replace the old buffer with the new buffer
            vertexBuffer = newBuffer;
            vertexBufferMemory = newBufferMemory;

            // Clean up the staging buffer
            vkDestroyBuffer(device, stagingBuffer, nullptr);
            vkFreeMemory(device, stagingBufferMemory, nullptr);

        } else {
            // Current buffer has enough space, append new data
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            createBuffer(newSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         stagingBuffer, stagingBufferMemory, device, physicalDevice);

            // Map and copy the new data to the staging buffer
            void* data;
            vkMapMemory(device, stagingBufferMemory, 0, newSize, 0, &data);
            memcpy(data, newVertices.data(), static_cast<size_t>(newSize));
            vkUnmapMemory(device, stagingBufferMemory);

            // Copy the new data from the staging buffer to the vertex buffer
            VkCommandBuffer commandBuffer = beginSingleTimeCommands(commandPool, device);
            VkBufferCopy copyRegion{};
            copyRegion.srcOffset = 0;
            copyRegion.dstOffset = currentSize; // Append the new data at the end of the existing data
            copyRegion.size = newSize;
            vkCmdCopyBuffer(commandBuffer, stagingBuffer, vertexBuffer, 1, &copyRegion);
            endSingleTimeCommands(commandBuffer, device, commandPool, graphicsQueue);

            // Clean up the staging buffer
            vkDestroyBuffer(device, stagingBuffer, nullptr);
            vkFreeMemory(device, stagingBufferMemory, nullptr);
        }
    }

/*
    void createUniformBuffers() {
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

        uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

        lightsBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        lightsBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
        lightsBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            createBuffer(bufferSize1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
            createBuffer(bufferSize2, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, lightsBuffers[i], lightsBuffersMemory[i]);

            vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize1, 0, &uniformBuffersMapped[i]);
            vkMapMemory(device, lightsBuffersMemory[i], 0, bufferSize2, 0, &lightsBuffersMapped[i]);
        }
    }
*/
    void createSkyBoxDescriptorPool() {
        std::array<VkDescriptorPoolSize, 5> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create sky box descriptor pool!");
        }
    }

    void createDescriptorPool() {
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = MAX_FRAMES_IN_FLIGHT * 4; // For three UBOs per frame
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = MAX_FRAMES_IN_FLIGHT * (texturePaths.size() + normalPaths.size() + 20);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.maxSets = MAX_FRAMES_IN_FLIGHT * 4; // Just an example, adjust accordingly

        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    void createTextDescriptorSets(){
        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, DSLText);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        descriptorSetsText.resize(MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSetsText.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorImageInfo imageInfo[2]{};
            imageInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo[0].imageView = TextImageView[0];
            imageInfo[0].sampler = TextSampler[0];
            imageInfo[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo[1].imageView = TextImageView[1];
            imageInfo[1].sampler = TextSampler[1];

            std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSetsText[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[0].descriptorCount = 2;
            descriptorWrites[0].pImageInfo = &imageInfo[0];

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    void updateTextDescriptorSets(){
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorImageInfo imageInfo[2]{};
            imageInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo[0].imageView = TextImageView[0];
            imageInfo[0].sampler = TextSampler[0];
            imageInfo[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo[1].imageView = TextImageView[1];
            imageInfo[1].sampler = TextSampler[1];

            std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSetsText[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[0].descriptorCount = 2;
            descriptorWrites[0].pImageInfo = &imageInfo[0];

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }


    void createSkyBoxDescriptorSets() {
        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, DSLskyBox);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        descriptorSetsSkyBox.resize(MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSetsSkyBox.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = skyboxImageView;
            imageInfo.sampler = skyboxSampler;

            VkDescriptorBufferInfo uniforBufferInfo{};
            uniforBufferInfo.buffer = uniformBuffers[i];
            uniforBufferInfo.offset = 0;
            uniforBufferInfo.range = sizeof(UniformBufferObject);

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSetsSkyBox[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pImageInfo = &imageInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSetsSkyBox[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pBufferInfo = &uniforBufferInfo;

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    void createMenuDescriptorSets(){
        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, DSLMenu);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        descriptorSetsMenu.resize(MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSetsMenu.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = menuImageView;
            imageInfo.sampler = menuSampler;

            std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSetsMenu[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    void createDescriptorSets() {
        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorBufferInfo bufferInfo1{};
            bufferInfo1.buffer = uniformBuffers[i];
            bufferInfo1.offset = 0;
            bufferInfo1.range = sizeof(UniformBufferObject);

            VkDescriptorBufferInfo bufferInfo2{};
            bufferInfo2.buffer = lightsBuffers[i];
            bufferInfo2.offset = 0;
            bufferInfo2.range = sizeof(LightsBufferObject);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = textureImageView;
            imageInfo.sampler = textureSampler;

            VkDescriptorBufferInfo matrixBufferInfo{};
            matrixBufferInfo.buffer = matrixUniformBuffers[i];
            matrixBufferInfo.offset = 0;
            matrixBufferInfo.range = sizeof(MatrixBufferObject);

            std::vector<VkDescriptorImageInfo> imageInfos;
            std::vector<VkDescriptorImageInfo> normalInfos;

            for (size_t j = 0; j < texturePaths.size(); ++j) {
                VkDescriptorImageInfo imageInfoO;

                imageInfoO.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfoO.imageView = textureImageViews[j];
                imageInfoO.sampler = textureSamplers[j];

                imageInfos.push_back(imageInfoO);
            }

            for (size_t j = 0; j < normalPaths.size(); ++j) {
                VkDescriptorImageInfo imageInfoO;

                imageInfoO.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfoO.imageView = normalImageViews[j];
                imageInfoO.sampler = normalSamplers[j];

                normalInfos.push_back(imageInfoO);
            }

            std::array<VkWriteDescriptorSet, 6> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo1;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[2].dstSet = descriptorSets[i];
            descriptorWrites[2].dstBinding = 2;
            descriptorWrites[2].dstArrayElement = 0;
            descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[2].descriptorCount = 1;
            descriptorWrites[2].pBufferInfo = &bufferInfo2;

            descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[3].dstSet = descriptorSets[i];
            descriptorWrites[3].dstBinding = 3;
            descriptorWrites[3].dstArrayElement = 0;
            descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[3].descriptorCount = 1;
            descriptorWrites[3].pBufferInfo = &matrixBufferInfo;

            descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[4].dstSet = descriptorSets[i];
            descriptorWrites[4].dstBinding = 4; // The same as the layout binding index
            descriptorWrites[4].dstArrayElement = 0;
            descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[4].descriptorCount = texturePaths.size();
            descriptorWrites[4].pImageInfo = imageInfos.data();

            descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[5].dstSet = descriptorSets[i];
            descriptorWrites[5].dstBinding = 5; // The same as the layout binding index
            descriptorWrites[5].dstArrayElement = 0;
            descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[5].descriptorCount = normalPaths.size();
            descriptorWrites[5].pImageInfo = normalInfos.data();

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    void createSyncObjects() {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    void drawFrame() {
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        //updateUniformBuffer(currentFrame, window, uniformBuffersMapped, lightsBuffersMapped);

        //updateUniformBuffer(currentFrame, window, uniformBuffersMapped, lightsBuffersMapped, normalProj);

        if(CurrentScene.index == ApplicationScene.index){
            updateMatrixUniformBuffer(currentFrame, listActualObjectInfos, matrixUniformBuffersMapped);
        }

        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);

        if(CurrentScene.index == ApplicationScene.index){
            recordCommandBufferApplication(commandBuffers[currentFrame], imageIndex, renderPass, swapChainFramebuffers,
                                           swapChainExtent, graphicsPipeline, vertexBuffer, indexBuffer, pipelineLayout,
                                           indices,
                                           descriptorSets,
                                           graphicsPipelineSkyBox, vertexBufferSB, indexBufferSB, pipelineLayoutSkyBox,
                                           skyBoxIndices, descriptorSetsSkyBox,
                                           graphicsPipelineText, vertexBufferText, indexBufferText, pipelineLayoutText,
                                           textIndices, descriptorSetsText,
                                           currentFrame);
        }else{
            recordCommandBufferMenu(commandBuffers[currentFrame], imageIndex, renderPass, swapChainFramebuffers,
                                    swapChainExtent, graphicsPipelineMenu, vertexBufferMenu, indexBufferMenu, pipelineLayoutMenu,
                                    menuIndices,
                                    descriptorSetsMenu,
                                    currentFrame);
        }


        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            recreateSwapChain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void createMenuVertexBuffer(std::vector<menuVertex> & vertices, VkBuffer & vertexBuffer, VkDeviceMemory & vertexBufferMemory){
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size() * 3;
        if(bufferSize == 0){
            bufferSize = 1;
        }

        VkDeviceSize actualBufferSize = sizeof(vertices[0]) * vertices.size();
        if(actualBufferSize == 0){
            actualBufferSize = 1;
        }

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     stagingBuffer, stagingBufferMemory, device, physicalDevice);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), (size_t) actualBufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                     vertexBuffer, vertexBufferMemory, device, physicalDevice);

        copyBuffer(stagingBuffer, vertexBuffer, bufferSize,
                   commandPool, device, graphicsQueue);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);

    }


    void createTextVertexBuffer(std::vector<textVertex> & vertices, VkBuffer & vertexBuffer, VkDeviceMemory & vertexBufferMemory){
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size() * 3;
        if(bufferSize == 0){
            bufferSize = 1;
        }

        VkDeviceSize actualBufferSize = sizeof(vertices[0]) * vertices.size();
        if(actualBufferSize == 0){
            actualBufferSize = 1;
        }

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     stagingBuffer, stagingBufferMemory, device, physicalDevice);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), (size_t) actualBufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                     vertexBuffer, vertexBufferMemory, device, physicalDevice);

        copyBuffer(stagingBuffer, vertexBuffer, bufferSize,
                   commandPool, device, graphicsQueue);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);

    }

    void createSBVertexBuffer(std::vector<skyBoxVertex> & vertices, VkBuffer & vertexBuffer, VkDeviceMemory & vertexBufferMemory) {
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size() * 3;
        if(bufferSize == 0){
            bufferSize = 1;
        }

        VkDeviceSize actualBufferSize = sizeof(vertices[0]) * vertices.size();
        if(actualBufferSize == 0){
            actualBufferSize = 1;
        }

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     stagingBuffer, stagingBufferMemory, device, physicalDevice);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), (size_t) actualBufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                     vertexBuffer, vertexBufferMemory, device, physicalDevice);

        copyBuffer(stagingBuffer, vertexBuffer, bufferSize,
                   commandPool, device, graphicsQueue);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }


    void createVertexBuffer(std::vector<Vertex> & vertices, VkBuffer & vertexBuffer, VkDeviceMemory & vertexBufferMemory) {
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
        if(bufferSize == 0){
            bufferSize = 1;
        }

        VkDeviceSize actualBufferSize = sizeof(vertices[0]) * vertices.size();
        if(actualBufferSize == 0){
            actualBufferSize = 1;
        }

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     stagingBuffer, stagingBufferMemory, device, physicalDevice);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), (size_t) actualBufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                     vertexBuffer, vertexBufferMemory, device, physicalDevice);

        copyBuffer(stagingBuffer, vertexBuffer, bufferSize,
                   commandPool, device, graphicsQueue);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }


    VkShaderModule createShaderModule(const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actualExtent = {
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    bool isDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices indices = findQueueFamilies(device);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return indices.isComplete() && extensionsSupported && swapChainAdequate  && supportedFeatures.samplerAnisotropy;
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (presentSupport) {
                indices.presentFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }

            i++;
        }

        return indices;
    }

    std::vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    /**
     * @brief Auxiliar function that help us in reading the binary files from the shaders that have been compiled,
     * it starts reading them from the end of the file so we can use the read position to determine the size of the
     * file and allocate a buffer. It reads all of the bytes from the specified file.
     * @param filename The name of the binary file that we want to open.
     * @return Return the bytes of the binary file in a byte array managed by std::vector.
     */
    static std::vector<char> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                        void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }
};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}