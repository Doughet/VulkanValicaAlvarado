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


void updateTransformationData(int pos, GLFWwindow * &window, std::vector<ObjectInformation*> listObjectInfos,
                              float &deltaTime) {
    const float moveSpeed = 5.0f;  // Units per second
    const float scaleSpeed = 0.5f; // Scale factor per second
    const float rotateSpeedDegrees = 35.0f; // Degrees per second
    const float rotateSpeedRadians = glm::radians(rotateSpeedDegrees); // Convert to radians per second

    std::cout << pos << std::endl;

    bool shiftPressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        if(shiftPressed) {
            listObjectInfos.at(pos)->modelMatrix = glm::translate(listObjectInfos.at(pos)->modelMatrix,
                                                                  glm::vec3(0.0f, moveSpeed * deltaTime, 0.0f));
        }else{
            listObjectInfos.at(pos)->modelMatrix = glm::translate( listObjectInfos.at(pos)->modelMatrix,
                                                                   glm::vec3(0.0f, -moveSpeed * deltaTime, 0.0f));
        }
    } else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        if(shiftPressed) {
            listObjectInfos.at(pos)->modelMatrix = glm::translate( listObjectInfos.at(pos)->modelMatrix,
                                                                   glm::vec3(-moveSpeed * deltaTime, 0.0f, 0.0f));
        }else{
            listObjectInfos.at(pos)->modelMatrix = glm::translate( listObjectInfos.at(pos)->modelMatrix,
                                                                   glm::vec3(moveSpeed * deltaTime, 0.0f, 0.0f));
        }
    } else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
        if(shiftPressed) {
            listObjectInfos.at(pos)->modelMatrix = glm::translate( listObjectInfos.at(pos)->modelMatrix,
                                                                   glm::vec3(0.0f, 0.0f, moveSpeed * deltaTime));
        }else{
            listObjectInfos.at(pos)->modelMatrix = glm::translate( listObjectInfos.at(pos)->modelMatrix,
                                                                   glm::vec3(0.0f, 0.0f,- moveSpeed * deltaTime));
        }
    } else if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
        if(shiftPressed) {
            listObjectInfos.at(pos)->modelMatrix = glm::scale(listObjectInfos.at(pos)->modelMatrix, glm::vec3(1.0f + scaleSpeed * deltaTime));
        }else{
            listObjectInfos.at(pos)->modelMatrix = glm::scale(listObjectInfos.at(pos)->modelMatrix, glm::vec3(1.0f - scaleSpeed * deltaTime));
        }
    }else if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
        if(shiftPressed) {
            listObjectInfos.at(pos)->modelMatrix = glm::rotate(listObjectInfos.at(pos)->modelMatrix, glm::radians(rotateSpeedDegrees * deltaTime), glm::vec3(0.0f, 0.0f, 1.0f));
        }else{
            listObjectInfos.at(pos)->modelMatrix = glm::rotate(listObjectInfos.at(pos)->modelMatrix, glm::radians(- rotateSpeedDegrees * deltaTime), glm::vec3(0.0f, 0.0f, 1.0f));
        }
    }
}

void changeCurrentModel(bool &keyPressed, GLFWwindow *&window, int &currentTransformationModel,
                        std::vector<ObjectInformation*> listObjectInfos){
    if(keyPressed == false) {
        if(glfwGetKey(window, GLFW_KEY_8) && currentTransformationModel != 0) {
            keyPressed = true;
            currentTransformationModel--;
        }else if (glfwGetKey(window, GLFW_KEY_9) && currentTransformationModel != listObjectInfos.size() - 1){
            keyPressed = true;
            currentTransformationModel++;
        }
    }
    if(!glfwGetKey(window, GLFW_KEY_9) && !glfwGetKey(window, GLFW_KEY_8)){
        keyPressed = false;
    }
}

void addObject(bool &keyPressedAdd, GLFWwindow *&window, uint32_t & selectedObject, bool &mustAdd){
    if(keyPressedAdd == false) {
        if(glfwGetKey(window, GLFW_KEY_Y)) {
            keyPressedAdd = true;
            selectedObject = 0;
            mustAdd = true;
        }else if (glfwGetKey(window, GLFW_KEY_U)){
            keyPressedAdd = true;
            selectedObject = 1;
            mustAdd = true;
        }else if (glfwGetKey(window, GLFW_KEY_I)){
            keyPressedAdd = true;
            selectedObject = 2;
            mustAdd = true;
        }
    }
    if(!glfwGetKey(window, GLFW_KEY_Y) && !glfwGetKey(window, GLFW_KEY_U) && !glfwGetKey(window, GLFW_KEY_I)){
        keyPressedAdd = false;
    }
}


// Control Wrapper
void handleGamePad(int id,  glm::vec3 &m, glm::vec3 &r, bool &fire) {
    const float deadZone = 0.1f;

    if(glfwJoystickIsGamepad(id)) {
        GLFWgamepadstate state;
        if (glfwGetGamepadState(id, &state)) {
            if(fabs(state.axes[GLFW_GAMEPAD_AXIS_LEFT_X]) > deadZone) {
                m.x += state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
            }
            if(fabs(state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]) > deadZone) {
                m.z += state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
            }
            if(fabs(state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER]) > deadZone) {
                m.y -= state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER];
            }
            if(fabs(state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER]) > deadZone) {
                m.y += state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER];
            }

            if(fabs(state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X]) > deadZone) {
                r.y += state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
            }
            if(fabs(state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]) > deadZone) {
                r.x += state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];
            }
            r.z += state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER] ? 1.0f : 0.0f;
            r.z -= state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] ? 1.0f : 0.0f;
            fire = fire | (bool)state.buttons[GLFW_GAMEPAD_BUTTON_A] | (bool)state.buttons[GLFW_GAMEPAD_BUTTON_B];
        }
    }
}

void getSixAxis(float &deltaT, glm::vec3 &m, glm::vec3 &r, bool &fire, GLFWwindow* &window) {
    static auto startTime = std::chrono::high_resolution_clock::now();
    static float lastTime = 0.0f;

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>
            (currentTime - startTime).count();
    deltaT = time - lastTime;
    lastTime = time;

    static double old_xpos = 0, old_ypos = 0;
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    double m_dx = xpos - old_xpos;
    double m_dy = ypos - old_ypos;
    old_xpos = xpos; old_ypos = ypos;

    const float MOUSE_RES = 10.0f;
    glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        r.y = -m_dx / MOUSE_RES;
        r.x = -m_dy / MOUSE_RES;
    }

    if(glfwGetKey(window, GLFW_KEY_LEFT)) {
        r.y = -1.0f;
    }
    if(glfwGetKey(window, GLFW_KEY_RIGHT)) {
        r.y = 1.0f;
    }
    if(glfwGetKey(window, GLFW_KEY_UP)) {
        r.x = -1.0f;
    }
    if(glfwGetKey(window, GLFW_KEY_DOWN)) {
        r.x = 1.0f;
    }
    if(glfwGetKey(window, GLFW_KEY_Q)) {
        r.z = 1.0f;
    }
    if(glfwGetKey(window, GLFW_KEY_E)) {
        r.z = -1.0f;
    }

    if(glfwGetKey(window, GLFW_KEY_A)) {
        m.x = -1.0f;
    }
    if(glfwGetKey(window, GLFW_KEY_D)) {
        m.x = 1.0f;
    }
    if(glfwGetKey(window, GLFW_KEY_S)) {
        m.z = 1.0f;
    }
    if(glfwGetKey(window, GLFW_KEY_W)) {
        m.z = -1.0f;
    }
    if(glfwGetKey(window, GLFW_KEY_R)) {
        m.y = 1.0f;
    }
    if(glfwGetKey(window, GLFW_KEY_F)) {
        m.y = -1.0f;
    }

    fire = glfwGetKey(window, GLFW_KEY_SPACE) | (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
    handleGamePad(GLFW_JOYSTICK_1,m,r,fire);
    handleGamePad(GLFW_JOYSTICK_2,m,r,fire);
    handleGamePad(GLFW_JOYSTICK_3,m,r,fire);
    handleGamePad(GLFW_JOYSTICK_4,m,r,fire);
}