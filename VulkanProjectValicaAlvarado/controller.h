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


#include "ObjectLoader.h"

/**
 * @brief Function that modifies a selected model from listObjectInfos by applying a trasnformation from the glm
 * library. If the user presses the 1 key it will make it move positively through the y axis. And if shift happens to
 * be pressed as well it will move negatively the model around the y axis as well. Same for the 2 and 3 keys but in
 * these cases for the x and z axis respectively. If the user presses the 4 key it will make the selected model smaller
 * but if shift is also pressed, it will make the  model bigger. If the user presses 5, it will rotate the model to the
 * right but if the sift key is pressed as well, the model will rotate counterclockwise.
 * The function calculates the speed of the diverse transformation factors by taking the deltaTime which will make that
 * transformations behave identically in different computers with diverse specifications.
 * @param pos It indicates the postion of the listObjectInfos so the user can modify the model he/she wishes.
 * @param window Receives as reference the window in which we are rendering the application we will use it for the
 * function *glfwGetKey(window, x)* that checks if for a specific window *window*, the x key is pressed.
 * @param listObjectInfos The list that contains the information of the different objects loaded on screen and it
 * contains as attribute the modelMatrix that needs to be modified for the  different transformations.
 * @param deltaTime It will vary on the performance of each PC/Laptop and it represents the difference between the
 * current moment and the last one the main loop was executed.
 */
void updateTransformationData(int pos, GLFWwindow * &window, std::vector<ObjectInformation*> listObjectInfos, float &deltaTime) {
    const float moveSpeed = 5.0f;  // Units per second
    const float scaleSpeed = 0.5f; // Scale factor per second
    const float rotateSpeedDegrees = 35.0f; // Degrees per second
    float sizeSpeed;

    const float fixedMoveSpeed = moveSpeed * deltaTime;

    const glm::mat4 M = listObjectInfos.at(pos)->modelMatrix;
    printf(", : %d ," ,int(listObjectInfos.at(pos)->vertices.size()));
    if(listObjectInfos.at(pos)->modelSize == modelSize::TINY){
        sizeSpeed = 0.1f;
    }else if(listObjectInfos.at(pos)->modelSize == modelSize::SMALL){
        sizeSpeed = 1.0f;
    }else if(listObjectInfos.at(pos)->modelSize == modelSize::MEDIUM){
        sizeSpeed = 1.8f;
    }else if(listObjectInfos.at(pos)->modelSize == modelSize::BIG){
        sizeSpeed = 3.5f;
    }else if(listObjectInfos.at(pos)->modelSize == modelSize::GIGANTIC){
        sizeSpeed = 8.5f;
    }else if(listObjectInfos.at(pos)->modelSize == modelSize::GLTF){
        sizeSpeed = 12.0f;
    }

    bool shiftPressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) { // y-axis movement
        if(shiftPressed) {
            listObjectInfos.at(pos)->modelMatrix = glm::translate(M, glm::vec3(0.0f, fixedMoveSpeed*sizeSpeed, 0.0f));
        }else{
            listObjectInfos.at(pos)->modelMatrix = glm::translate( M,glm::vec3(0.0f, - fixedMoveSpeed*sizeSpeed, 0.0f));
        }
    } else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) { // x-axis movement
        if(shiftPressed) {
            listObjectInfos.at(pos)->modelMatrix = glm::translate( M, glm::vec3(- fixedMoveSpeed*sizeSpeed, 0.0f, 0.0f));
        }else{
            listObjectInfos.at(pos)->modelMatrix = glm::translate( M, glm::vec3(fixedMoveSpeed*sizeSpeed, 0.0f, 0.0f));
        }
    } else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) { // z-axis movement
        if(shiftPressed) {
            listObjectInfos.at(pos)->modelMatrix = glm::translate( M, glm::vec3(0.0f, 0.0f, fixedMoveSpeed*sizeSpeed));
        }else{
            listObjectInfos.at(pos)->modelMatrix = glm::translate( M, glm::vec3(0.0f, 0.0f,- fixedMoveSpeed*sizeSpeed));
        }
    } else if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) { // Scaling section
        float fixedScaleSpeed = scaleSpeed * deltaTime;
        if(shiftPressed) {
            listObjectInfos.at(pos)->modelMatrix = glm::scale(M, glm::vec3(1.0f + fixedScaleSpeed));
        }else{
            listObjectInfos.at(pos)->modelMatrix = glm::scale(M, glm::vec3(1.0f - fixedScaleSpeed));
        }
    }else if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) { // Rotating
        float fixedRotateSpeed = glm::radians(rotateSpeedDegrees * deltaTime);
        if (shiftPressed) {
            listObjectInfos.at(pos)->modelMatrix = glm::rotate(M, fixedRotateSpeed, glm::vec3(0.0f, 0.0f, 1.0f));
        } else {
            listObjectInfos.at(pos)->modelMatrix = glm::rotate(M, - fixedRotateSpeed, glm::vec3(0.0f, 0.0f, 1.0f));
        }
    }
}


/**
 * @brief This function allows the user to select a model from listObjectInfos to modify it later in other functions.
 * The 8 key decreases the currentTransformationModel by one if it is not already 0 and the 9 key increases it if it is
 * smaller than the size listObjectInfos - 1.
 * @param keyPressed Boolean variable that checks whether the 8 or 9 keys were already pressed so the
 * currentTransformationModel can only increase or decrease by one. If that is the case  it becomes true, if none of
 * them are pressed it is updated to false.
 * @param window Receives as reference the window in which we are rendering the application we will use it for the
 * function *glfwGetKey(window, x)* that checks if for a specific window *window*, the x key is pressed.
 * @param currentTransformationModel the index that accesses the object of listObjectsInfos that is needed.
 * @param listObjectInfos The list that contains the information of the different objects loaded on screen and it
 * contains as attribute the modelMatrix that needs to be modified for the  different transformations.
 */
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
    if(!glfwGetKey(window, GLFW_KEY_9) && !glfwGetKey(window, GLFW_KEY_8) && listObjectInfos.at(currentTransformationModel)->isGltf){
        if(listObjectInfos.at(currentTransformationModel)->vertices.size() <= 2000){
            listObjectInfos.at(currentTransformationModel)->modelSize = modelSize::TINY;
        }else if(listObjectInfos.at(currentTransformationModel)->vertices.size() <= 4000){
            listObjectInfos.at(currentTransformationModel)->modelSize = modelSize::SMALL;
        }else if(listObjectInfos.at(currentTransformationModel)->vertices.size() <= 6000){
            listObjectInfos.at(currentTransformationModel)->modelSize = modelSize::MEDIUM;
        }else if(listObjectInfos.at(currentTransformationModel)->vertices.size() <= 7500){
            listObjectInfos.at(currentTransformationModel)->modelSize = modelSize::BIG;
        }else if(listObjectInfos.at(currentTransformationModel)->vertices.size() <= 1000000){
            listObjectInfos.at(currentTransformationModel)->modelSize = modelSize::GIGANTIC;
        }
        keyPressed = false;
    }else if(!glfwGetKey(window, GLFW_KEY_9) && !glfwGetKey(window, GLFW_KEY_8)){
        listObjectInfos.at(currentTransformationModel)->modelSize = modelSize::GLTF;
        keyPressed = false;
    }
}

/**
 * @brief
 * @param keyPressedAdd
 * @param window
 * @param selectedObject
 * @param mustAdd
 */
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
        }/*else if (glfwGetKey(window, GLFW_KEY_I)){
            keyPressedAdd = true;
            selectedObject = 2;
            mustAdd = true;
        }*/else if (glfwGetKey(window, GLFW_KEY_X)) {
            keyPressedAdd = true;
            selectedObject = 3;
            mustAdd = true;
        }
    }
    if(!glfwGetKey(window, GLFW_KEY_Y) && !glfwGetKey(window, GLFW_KEY_U) && !glfwGetKey(window, GLFW_KEY_I)){
        keyPressedAdd = false;
    }
}

void deleteObject(bool &keyPressedDelete, GLFWwindow *&window, bool &mustDelete){
    if(keyPressedDelete == false) {
        if(glfwGetKey(window, GLFW_KEY_BACKSPACE)) {
            keyPressedDelete = true;
            mustDelete = true;
        }
    }
    if(!glfwGetKey(window, GLFW_KEY_BACKSPACE)){
        keyPressedDelete = false;
    }
}

/**
 *
 * @param id
 * @param m
 * @param r
 * @param fire
 */
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

/**
 * @brief
 * @param deltaT
 * @param m
 * @param r
 * @param fire
 * @param window
 */
void getSixAxis(float &deltaT, glm::vec3 &m, glm::vec3 &r, bool &fire, GLFWwindow* &window, bool &normalProj) {
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
    if(normalProj == true) {
        glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            r.y = -m_dx / MOUSE_RES;
            r.x = -m_dy / MOUSE_RES;
        }
    }

    if(normalProj == true) {
        if (glfwGetKey(window, GLFW_KEY_LEFT)) {
            r.y = -1.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT)) {
            r.y = 1.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_UP)) {
            r.x = -1.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN)) {
            r.x = 1.0f;
        }
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
    if(glfwGetKey(window, GLFW_KEY_R)) { // Does not work
        m.y = 1.0f;
    }
    if(glfwGetKey(window, GLFW_KEY_F)) { // Does not work
        m.y = -1.0f;
    }
    if(normalProj == true) {
        fire = glfwGetKey(window, GLFW_KEY_SPACE) | (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
        handleGamePad(GLFW_JOYSTICK_1, m, r, fire);
        handleGamePad(GLFW_JOYSTICK_2, m, r, fire);
        handleGamePad(GLFW_JOYSTICK_3, m, r, fire);
        handleGamePad(GLFW_JOYSTICK_4, m, r, fire);
    }
}