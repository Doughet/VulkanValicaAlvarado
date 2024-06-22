//
// Created by adria on 22/06/2024.
//
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <vulkan/vulkan.h>

#ifndef VULKANPROJECTVALICAALVARADO_CHARINFOSTRUCTURE_H
#define VULKANPROJECTVALICAALVARADO_CHARINFOSTRUCTURE_H

#endif //VULKANPROJECTVALICAALVARADO_CHARINFOSTRUCTURE_H
#include <fstream>
#include <sstream>


struct CharInfo {
    uint32_t id;
    float x, y;
    float width, height;
    float xoffset, yoffset;
    float xadvance;
};

std::unordered_map<uint32_t, CharInfo> parseFNT(const std::string& filename) {
    std::unordered_map<uint32_t, CharInfo> charMap;
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open FNT file.");
    }

    std::string line;
    while (std::getline(file, line)) {
        // Check if the line contains a <char> tag
        if (line.find("<char") != std::string::npos) {
            CharInfo ci;
            size_t pos = 0;

            // Extract the id attribute
            if ((pos = line.find("id=\"")) != std::string::npos) {
                size_t start = pos + 4;
                size_t end = line.find("\"", start);
                ci.id = std::stoi(line.substr(start, end - start));
            }

            // Extract the x attribute
            if ((pos = line.find("x=\"")) != std::string::npos) {
                size_t start = pos + 3;
                size_t end = line.find("\"", start);
                ci.x = std::stoi(line.substr(start, end - start));
            }

            // Extract the y attribute
            if ((pos = line.find("y=\"")) != std::string::npos) {
                size_t start = pos + 3;
                size_t end = line.find("\"", start);
                ci.y = std::stoi(line.substr(start, end - start));
            }

            // Extract the width attribute
            if ((pos = line.find("width=\"")) != std::string::npos) {
                size_t start = pos + 7;
                size_t end = line.find("\"", start);
                ci.width = std::stoi(line.substr(start, end - start));
            }

            // Extract the height attribute
            if ((pos = line.find("height=\"")) != std::string::npos) {
                size_t start = pos + 8;
                size_t end = line.find("\"", start);
                ci.height = std::stoi(line.substr(start, end - start));
            }

            // Extract the xoffset attribute
            if ((pos = line.find("xoffset=\"")) != std::string::npos) {
                size_t start = pos + 9;
                size_t end = line.find("\"", start);
                ci.xoffset = std::stoi(line.substr(start, end - start));
            }

            // Extract the yoffset attribute
            if ((pos = line.find("yoffset=\"")) != std::string::npos) {
                size_t start = pos + 9;
                size_t end = line.find("\"", start);
                ci.yoffset = std::stoi(line.substr(start, end - start));
            }

            // Extract the xadvance attribute
            if ((pos = line.find("xadvance=\"")) != std::string::npos) {
                size_t start = pos + 10;
                size_t end = line.find("\"", start);
                ci.xadvance = std::stoi(line.substr(start, end - start));
            }

            charMap[ci.id] = ci;
        }
    }

    return charMap;
}
