#include <glad/glad.h>

#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

#include "../include/glm/glm.hpp"
#include "../include/glm/gtc/matrix_transform.hpp"
#include "../include/glm/gtc/type_ptr.hpp"

#include "../include/shader.h"

#include <fstream>
#include <iostream>
#include <math.h>
#include <sstream>

#define ARRAY_COUNT(array)                                                                         \
    (sizeof(array) /                                                                               \
     (sizeof(array[0]) * (sizeof(array) != sizeof(void*) || sizeof(array[0]) <= sizeof(void*))))

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// Window
const unsigned int SCR_WIDTH = 400;
const unsigned int SCR_HEIGHT = 400;
GLFWwindow* window;

// Shader
Shader* shader;

// Time
float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

// Camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
const float CAMERA_SPEED = 8.0f;
const float MOUSE_SENSITIVITY = 0.03;
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
float fov = 45.0f;

// Render
GLuint vertex_buffer_object;
GLuint index_buffer_object;
GLuint vertex_array_object;
const GLshort index_data[] = {0, 1, 2, 3, 4};
const float vertex_data[] = {
    // positions
    // triangle A
    1.0f, 0.0f, -1.0f, // bottom right
    0.0f, 0.0f, -1.0f, // bottom left
    0.0f, 1.0f, -1.0f, // top
    // line A
    0.0f, 0.0f, 0.0f, // start
    1.0f, 1.0f, 0.0f, // end

    // colors
    // triangle A
    1.0f, 0.0f, 0.0f, // bottom right
    0.0f, 1.0f, 0.0f, // bottom left
    0.0f, 0.0f, 1.0f, // top
    // line A
    1.0f, 0.0f, 0.0f, // start
    0.0f, 1.0f, 0.0f, // end
};
const int vertex_count = sizeof(vertex_data) / sizeof(float) / (3 + 3);
const int line_index_offset = 3;

// Adjust camera direction
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    xoffset *= MOUSE_SENSITIVITY;
    yoffset *= MOUSE_SENSITIVITY;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

// Adjust camera zoom
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (fov >= 1.0f && fov <= 45.0f)
        fov -= yoffset;
    if (fov <= 1.0f)
        fov = 1.0f;
    if (fov >= 45.0f)
        fov = 45.0f;
}

int init_program() {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // glfw window creation
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Jaragayt", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    return 0;
}

void init_vertex_buffer() {
    glGenBuffers(1, &vertex_buffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &index_buffer_object);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_object);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_data), index_data, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void init_vertices() {
    glGenVertexArrays(1, &vertex_array_object);
    glBindVertexArray(vertex_array_object);

    size_t color_data_offset = sizeof(float) * 3 * vertex_count;
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0,        // attribute 0 in shader
                          3,        // size
                          GL_FLOAT, // type
                          GL_FALSE, // normalized?
                          0,        // stride
                          (void*)0  // array buffer offset
    );
    glVertexAttribPointer(1,                       // atrtribute 1 in shader
                          3,                       // size
                          GL_FLOAT,                // type
                          GL_FALSE,                // normalized?
                          0,                       // stride
                          (void*)color_data_offset // array buffer offset
    );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_object);

    glBindVertexArray(0);
}

// Initalize shaders
void init_shaders() {
    shader = new Shader("assets/shaders/vert.glsl", "assets/shaders/frag.glsl");
    shader->use();
}

void draw() {
    glBindVertexArray(vertex_array_object);

    // Draw triangles
    glDrawElements(GL_TRIANGLES, ARRAY_COUNT(index_data), GL_UNSIGNED_SHORT, 0);

    // Draw lines
    glDrawElementsBaseVertex(GL_LINES, 2, GL_UNSIGNED_SHORT, 0, line_index_offset);
}

void view_projection_model() {
    // activate shader
    shader->use();

    // camera/view transformation
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    shader->setMat4("view", view);

    // projection
    glm::mat4 projection =
        glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    shader->setMat4("projection", projection);

    // model
    glm::mat4 model = glm::mat4(1.0f);
    shader->setMat4("model", model);
}

int main() {
    // setup
    init_program();
    init_vertex_buffer();
    init_shaders();
    init_vertices();
    // setup_triangle_vertices();
    // setup_line_vertices();

    // render loop
    while (!glfwWindowShouldClose(window)) {
        // update time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // render
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render
        view_projection_model();
        draw();

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // deallocated shader
    delete shader;

    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}

// process all input
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = CAMERA_SPEED * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraPos += cameraSpeed * cameraFront;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraPos -= cameraSpeed * cameraFront;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
