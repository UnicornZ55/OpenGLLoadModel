#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>
#include <vector>

// ==== CALLBACKS ====
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// ==== SETTINGS ====
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// ==== CAMERA ====
Camera camera(glm::vec3(0.0f, 3.0f, 8.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// ==== TIMING ====
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// ==== PLAYER ====
glm::vec3 playerPos = glm::vec3(0.0f, 0.5f, 0.0f);
float playerYaw = 0.0f;

// ==== CUBE POSITIONS ====
std::vector<glm::vec3> cubePositions = {
    {0.0f, 0.5f, -3.0f},
    {2.0f, 0.5f, -5.0f},
    {-2.0f, 0.5f, -4.0f}
};
glm::vec3 cubeSize = glm::vec3(1.0f, 1.0f, 1.0f);

// ==== COLLISION CHECK ====
bool checkCollision(glm::vec3 aPos, glm::vec3 aSize, glm::vec3 bPos, glm::vec3 bSize)
{
    return (abs(aPos.x - bPos.x) < (aSize.x + bSize.x) / 2) &&
        (abs(aPos.y - bPos.y) < (aSize.y + bSize.y) / 2) &&
        (abs(aPos.z - bPos.z) < (aSize.z + bSize.z) / 2);
}

// ==== DRAW CUBE ====
unsigned int cubeVAO = 0, cubeVBO = 0;
void drawCube()
{
    if (cubeVAO == 0)
    {
        float vertices[] = {
            -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f,0.5f,-0.5f,  0.5f,0.5f,-0.5f, -0.5f,0.5f,-0.5f, -0.5f,-0.5f,-0.5f,
            -0.5f,-0.5f, 0.5f,  0.5f,-0.5f, 0.5f,  0.5f,0.5f, 0.5f,  0.5f,0.5f, 0.5f, -0.5f,0.5f, 0.5f, -0.5f,-0.5f, 0.5f,
            -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,-0.5f, -0.5f,-0.5f,-0.5f, -0.5f,-0.5f,-0.5f, -0.5f,-0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
             0.5f, 0.5f, 0.5f,  0.5f, 0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f,-0.5f, 0.5f,  0.5f, 0.5f, 0.5f,
            -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f,-0.5f, 0.5f,  0.5f,-0.5f, 0.5f, -0.5f,-0.5f, 0.5f, -0.5f,-0.5f,-0.5f,
            -0.5f, 0.5f,-0.5f,  0.5f, 0.5f,-0.5f,  0.5f, 0.5f, 0.5f,  0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,-0.5f
        };

        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindVertexArray(cubeVAO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// ==== MAIN ====
int main()
{
    // --- Init GLFW & OpenGL ---
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Simple 3D Game", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    stbi_set_flip_vertically_on_load(false);
    glEnable(GL_DEPTH_TEST);

    // --- Load Shaders ---
    Shader ourShader(
        "C:/Users/asus/Downloads/JoeyDeVries-LearnOpenGL-a545a70/JoeyDeVries-LearnOpenGL-a545a70/src/3.model_loading/1.model_loading/1.model_loading.vs",
        "C:/Users/asus/Downloads/JoeyDeVries-LearnOpenGL-a545a70/JoeyDeVries-LearnOpenGL-a545a70/src/3.model_loading/1.model_loading/1.model_loading.fs"
    );

    Shader groundShader(
        "C:/Users/asus/Downloads/JoeyDeVries-LearnOpenGL-a545a70/JoeyDeVries-LearnOpenGL-a545a70/src/3.model_loading/1.model_loading/ground.vs",
        "C:/Users/asus/Downloads/JoeyDeVries-LearnOpenGL-a545a70/JoeyDeVries-LearnOpenGL-a545a70/src/3.model_loading/1.model_loading/ground.fs"
    );

    // --- Load Model ---
    Model ourModel("C:/Users/asus/Downloads/JoeyDeVries-LearnOpenGL-a545a70/JoeyDeVries-LearnOpenGL-a545a70/src/3.model_loading/1.model_loading/milk_carton.obj");

    // --- Ground Plane ---
    float groundVertices[] = {
        -20.0f, 0.0f, -20.0f,
         20.0f, 0.0f, -20.0f,
         20.0f, 0.0f,  20.0f,
        -20.0f, 0.0f,  20.0f
    };
    unsigned int groundIndices[] = { 0, 1, 2, 0, 2, 3 };

    unsigned int groundVAO, groundVBO, groundEBO;
    glGenVertexArrays(1, &groundVAO);
    glGenBuffers(1, &groundVBO);
    glGenBuffers(1, &groundEBO);

    glBindVertexArray(groundVAO);
    glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(groundIndices), groundIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // --- Game Loop ---
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // --- Camera follows player ---
        float followDistance = 2.0f;
        float followHeight = 0.5f;
        camera.Position = playerPos - camera.Front * followDistance + glm::vec3(0.0f, followHeight, 0.0f);

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
            (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // --- Draw Ground ---
        groundShader.use();
        groundShader.setMat4("projection", projection);
        groundShader.setMat4("view", view);

        glm::mat4 groundModel = glm::mat4(1.0f);
        groundShader.setMat4("model", groundModel);
        groundShader.setVec3("color", glm::vec3(0.1f, 0.8f, 0.1f)); // green
        glBindVertexArray(groundVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // --- Draw All Cubes ---
        for (auto& pos : cubePositions)
        {
            bool collided = checkCollision(playerPos, glm::vec3(1.0f), pos, cubeSize);

            groundShader.use();
            groundShader.setMat4("projection", projection);
            groundShader.setMat4("view", view);

            // red cube
            groundShader.setVec3("color", glm::vec3(0.9f, 0.1f, 0.1f));

            glm::mat4 cubeModel = glm::mat4(1.0f);
            cubeModel = glm::translate(cubeModel, pos);
            cubeModel = glm::scale(cubeModel, glm::vec3(1.0f));
            groundShader.setMat4("model", cubeModel);
            drawCube();

            if (collided)
                std::cout << "Hit!!!!" << std::endl;
        }

        // --- Draw Player ---
        ourShader.use();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, playerPos);
        model = glm::rotate(model, glm::radians(playerYaw), glm::vec3(0.0f, 1.0f, 0.0f));
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// ==== INPUT ====
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float speed = 5.0f * deltaTime;
    glm::vec3 forward = glm::normalize(glm::vec3(camera.Front.x, 0.0f, camera.Front.z));
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        playerPos += forward * speed;
        playerYaw = glm::degrees(atan2(forward.x, forward.z));
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        playerPos -= forward * speed;
        playerYaw = glm::degrees(atan2(-forward.x, -forward.z));
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        playerPos -= right * speed;
        playerYaw = glm::degrees(atan2(-right.x, -right.z));
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        playerPos += right * speed;
        playerYaw = glm::degrees(atan2(right.x, right.z));
    }

    playerPos.y = 0.5f; // keep on ground
}

// ==== CALLBACKS ====
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
