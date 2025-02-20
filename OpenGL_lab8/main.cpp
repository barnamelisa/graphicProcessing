#if defined (APPLE)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/matrix_inverse.hpp>
#include <gtc/type_ptr.hpp>
#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include <iostream>
#include "SkyBox.hpp"

int glWindowWidth = 1900;
int glWindowHeight = 1050;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

// matrici si variabile globale
glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;

// lumina
glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

GLfloat angle;

// Obiectele scenei
gps::Camera myCamera(
    glm::vec3(0.0f, 2.0f, 5.5f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.2f;

bool pressedKeys[1024];
double lastX = 400, lastY = 300;
bool firstMouse = true;
gps::Model3D scena;
gps::Shader myCustomShader;

// FBO pentru umbra
GLuint shadowMapFBO;
GLuint depthMapTexture;

gps::SkyBox mySkyBox;
gps::Shader skyboxShader;
gps::Shader shadowShader;

bool oKeyPressed = false;        // verifica daca tasta "O" a fost apasata
float movementTimer = 0.0f;      // contor pentru timpul care a trecut de la apasarea tastei "O"
bool movementInProgress = false; // verifica daca miscarile sunt deja în curs
const float movementDuration = 3.0f;  // durata totala a secventei de miscari 
const float moveInterval = 1.0f;      // intervalul intre miscari (in secunde)
const float timerIncrement = 0.1f;

bool useDirectionalLight = true;

//lumina punctiforma1
glm::vec3 pointLight1 = glm::vec3(-10.334f, 9.77501f, 0.754904f); //  aici pun ce coord are lumina 
GLuint pointLightLoc1;
glm::vec3 pointLightColor1 = glm::vec3(1.0f, 1.0f, 1.0f);
GLuint pointLightColorLoc1;


GLenum glCheckError_(const char* file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
    //TODO
}

float initialPitchOffset = 0.0f;
float initialYawOffset = 0.0f;

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {

    float xOffset = xpos - lastX;
    float yOffset = lastY - ypos;  
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    initialYawOffset += xOffset;
    initialPitchOffset += yOffset;

    if (initialPitchOffset > 89.0f) initialPitchOffset = 89.0f;
    if (initialPitchOffset < -89.0f) initialPitchOffset = -89.0f;

    myCamera.rotate(initialPitchOffset, initialYawOffset);

    view = myCamera.getViewMatrix();
    myCustomShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

void initSkybox() {
    std::vector<const GLchar*> faces;

    faces.push_back("skybox/munti/posx.jpg");
    faces.push_back("skybox/munti/negx.jpg");
    faces.push_back("skybox/munti/posy.jpg");
    faces.push_back("skybox/munti/negy.jpg");
    faces.push_back("skybox/munti/posz.jpg");
    faces.push_back("skybox/munti/negz.jpg");
 
    mySkyBox.Load(faces);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
        useDirectionalLight = !useDirectionalLight;
        std::cout << "Light mode changed to: " << (useDirectionalLight ? "Directional Light" : "Other Light") << std::endl;
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)
            pressedKeys[key] = true;
        else if (action == GLFW_RELEASE)
            pressedKeys[key] = false;
    }
}

void processMovement() {
    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
    }

    // modul solid = obiectele vor fi desenate sub forma de linii cu suprafetele solide vizibile
    if (pressedKeys[GLFW_KEY_H]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // wireframe = obiectele vor fi desenate sub forma de linii, evidentiind doar conturul
    if (pressedKeys[GLFW_KEY_J]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    // poligonal = obiecte desenate doar prin varfuri, vizibil pt a evidentia doar doar pozitiile varfurilor
    if (pressedKeys[GLFW_KEY_K]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }

    // smooth = suprafetele obiectelor vor fi desenate neted
    if (pressedKeys[GLFW_KEY_L]) {
        glShadeModel(GL_SMOOTH);
    }

    if (pressedKeys[GLFW_KEY_O] && !oKeyPressed) {
        oKeyPressed = true;           
        movementTimer = 0.0f;         
        movementInProgress = true;    
    }

    if (pressedKeys[GLFW_KEY_Y] &&!oKeyPressed) {
        useDirectionalLight = !useDirectionalLight; 
        std::cout << "Light mode changed to: " << (useDirectionalLight ? "Directional Light" : "Point Light") << std::endl;

        glm::vec3 cameraPos = myCamera.getPosition();
        std::cout << "Pozitia camerei: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")\n";

        GLuint useDirectionalLightLoc = glGetUniformLocation(myCustomShader.shaderProgram, "useDirectionalLight");
        glUniform1i(useDirectionalLightLoc, useDirectionalLight ? 1 : 0);

        // actualizez lumina pt shader
        if (useDirectionalLight) {

            // lumina directionala
            glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
            glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

            glUniform3fv(pointLightLoc1, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, 0.0f)));  
            glUniform3fv(pointLightColorLoc1, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, 0.0f)));  
        }
        else {
            // lumina directionala
            glUniform3fv(pointLightLoc1, 1, glm::value_ptr(pointLight1));
            glUniform3fv(pointLightColorLoc1, 1, glm::value_ptr(pointLightColor1));

            glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, 0.0f)));  
            glUniform3fv(lightColorLoc, 1, glm::value_ptr(glm::vec3(0.1f, 0.1f, 0.1f)));  
        }

    }

    // pentru animatii: mai multe miscari in continuu pentru a prezenta scena
    if (movementInProgress) {
       
        if (movementTimer < moveInterval) {
            myCamera.move(gps::MOVE_FORWARD, 0.3f);  
        }
        else if (movementTimer >= moveInterval && movementTimer < 2 * moveInterval) {
            myCamera.move(gps::MOVE_LEFT, 0.2f);     
        }
        else if (movementTimer >= 2 * moveInterval && movementTimer < 3 * moveInterval) {
            myCamera.move(gps::MOVE_FORWARD, 0.3f);  
        }
        else if (movementTimer >= 3 * moveInterval && movementTimer < 4 * moveInterval) {
            myCamera.move(gps::MOVE_RIGHT, 0.2f);    
        }
        else if (movementTimer >= 4 * moveInterval && movementTimer < 5 * moveInterval) {
            myCamera.move(gps::MOVE_FORWARD, 0.3f);  
        }
        else if (movementTimer >= 5 * moveInterval && movementTimer < 6 * moveInterval) {
            myCamera.move(gps::MOVE_LEFT, 0.2f);    
        }
        else if (movementTimer >= 6 * moveInterval && movementTimer < 7 * moveInterval) {
            myCamera.move(gps::MOVE_FORWARD, 0.3f); 
        }
        else if (movementTimer >= 7 * moveInterval && movementTimer < 8 * moveInterval) {
            myCamera.move(gps::MOVE_RIGHT, 0.2f);   
        }
        else if (movementTimer >= 8 * moveInterval && movementTimer < 9 * moveInterval) {
            myCamera.move(gps::MOVE_BACKWARD, 0.15f); 
        }
        else if (movementTimer >= 9 * moveInterval && movementTimer < 10 * moveInterval) {
            myCamera.move(gps::MOVE_FORWARD, 0.3f);  
        }
        else if (movementTimer >= 10 * moveInterval && movementTimer < 11 * moveInterval) {
            myCamera.move(gps::MOVE_LEFT, 0.2f);     
        }
        else if (movementTimer >= 11 * moveInterval && movementTimer < 12 * moveInterval) {
            myCamera.move(gps::MOVE_FORWARD, 0.3f);  
        }
        else if (movementTimer >= 12 * moveInterval && movementTimer < 13 * moveInterval) {
            myCamera.move(gps::MOVE_RIGHT, 0.2f);    
        }
        else if (movementTimer >= 13 * moveInterval && movementTimer < 14 * moveInterval) {
            myCamera.move(gps::MOVE_FORWARD, 0.3f);  
        }
        else if (movementTimer >= 14 * moveInterval && movementTimer < 15 * moveInterval) {
            myCamera.move(gps::MOVE_LEFT, 0.2f);     
        }
        else if (movementTimer >= 15 * moveInterval && movementTimer < 16 * moveInterval) {
            myCamera.move(gps::MOVE_FORWARD, 0.3f);  
        }
        else if (movementTimer >= 16 * moveInterval && movementTimer < 17 * moveInterval) {
            myCamera.move(gps::MOVE_RIGHT, 0.2f);   
        }
        else if (movementTimer >= 17 * moveInterval && movementTimer < 18 * moveInterval) {
            myCamera.move(gps::MOVE_BACKWARD, 0.15f); 
        }
        else if (movementTimer >= 18 * moveInterval && movementTimer < 19 * moveInterval) {
            myCamera.move(gps::MOVE_FORWARD, 0.3f);  
        }
        else if (movementTimer >= 19 * moveInterval && movementTimer < 20 * moveInterval) {
            myCamera.move(gps::MOVE_LEFT, 0.2f);     
        }
        else {
            movementInProgress = false; 
        }

        movementTimer += timerIncrement;  
    }

    if (!pressedKeys[GLFW_KEY_O]) {
        oKeyPressed = false;
    }
}

bool initOpenGLWindow() {
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Scene", NULL, NULL);
    if (!glWindow) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return false;
    }

    glfwSetKeyCallback(glWindow, keyboardCallback);
    glfwSetCursorPosCallback(glWindow, mouseCallback);
    glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    glfwMakeContextCurrent(glWindow);
    glfwSwapInterval(1);

#if not defined (APPLE)
    glewExperimental = GL_TRUE;
    glewInit();
#endif

    glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);
    return true;
}

void initOpenGLState() {
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glViewport(0, 0, retina_width, retina_height);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}

void initObjects() {
    scena.LoadModel("objects/LowPolyModels/scena_var17.obj");
}

void initShaders() {
    myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
    myCustomShader.useShaderProgram();

    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();
}

void initUniforms() {
    myCustomShader.useShaderProgram();

    // matrice model
    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // matrice view
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // matrice projection
    projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // lumina directionala 
    glm::vec3 dirLightDir = glm::normalize(glm::vec3(0.0f, -1.0f, -0.5f));  
    GLuint dirLightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPosEye1");
    glUniform3fv(dirLightDirLoc, 1, glm::value_ptr(dirLightDir));

    glm::vec3 dirLightColor = glm::vec3(1.0f, 1.0f, 1.0f); 
    GLuint dirLightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor1");
    glUniform3fv(dirLightColorLoc, 1, glm::value_ptr(dirLightColor));

    // lumina ambientala globala
    lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
    lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

    lightColor = glm::vec3(1.5f, 1.5f, 1.5f);
    lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    glm::vec3 lightPos1 = myCamera.getViewMatrix() * glm::vec4(5.0f, 5.0f, 5.0f, 1.0f);  
    glm::vec3 lightPos2 = myCamera.getViewMatrix() * glm::vec4(-5.0f, 5.0f, 5.0f, 1.0f);
    glm::vec3 lightPos3 = myCamera.getViewMatrix() * glm::vec4(0.0f, -5.0f, 5.0f, 1.0f);

    glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightPos1"), 1, glm::value_ptr(lightPos1));
    glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightPos2"), 1, glm::value_ptr(lightPos2));
    glUniform3fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightPos3"), 1, glm::value_ptr(lightPos3));

    // pt lumina punctiforma
    pointLightLoc1 = glGetUniformLocation(myCustomShader.shaderProgram, "pointLight1");
    glUniform3fv(pointLightLoc1, 1, glm::value_ptr(pointLight1));

    //set point light color
    pointLightColorLoc1 = glGetUniformLocation(myCustomShader.shaderProgram, "pointLightColor1");
    glUniform3fv(pointLightColorLoc1, 1, glm::value_ptr(pointLightColor1));

    initSkybox();
}

void initFBO() {
    glGenFramebuffers(1, &shadowMapFBO);
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void drawObjects(gps::Shader shader) {
    shader.useShaderProgram();
    model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    scena.Draw(shader);
}

void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDepthMask(GL_FALSE); 
    skyboxShader.useShaderProgram();
    glm::mat4 skyboxView = glm::mat4(glm::mat3(view)); 
    mySkyBox.Draw(skyboxShader, skyboxView, projection);
    glDepthMask(GL_TRUE); 

    myCustomShader.useShaderProgram();
    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    drawObjects(myCustomShader);
}

void cleanup() {
    glDeleteTextures(1, &depthMapTexture);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &shadowMapFBO);
    glfwDestroyWindow(glWindow);
    glfwTerminate();
}

int main(int argc, const char* argv[]) {
    if (!initOpenGLWindow()) {
        glfwTerminate();
        return 1;
    }

    initOpenGLState();
    initObjects();
    initShaders();
    initUniforms();
    initFBO();

    while (!glfwWindowShouldClose(glWindow)) {
        processMovement();
        renderScene();
        glfwPollEvents();
        glfwSwapBuffers(glWindow);
    }

    cleanup();
    return 0;
}