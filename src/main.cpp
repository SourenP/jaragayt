#include <glad/glad.h>

#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

#include "../include/glm/glm.hpp"
#include "../include/glm/gtc/matrix_transform.hpp"
#include "../include/glm/gtc/type_ptr.hpp"

#include "../include/shader.h"
#include "camera.hpp"
#include "constants.hpp"
#include "geometry.hpp"

#include <array>
#include <fstream>
#include <iostream>
#include <math.h>
#include <sstream>
#include <vector>

#define ARRAY_COUNT(array)                                                                         \
    (sizeof(array) /                                                                               \
     (sizeof(array[0]) * (sizeof(array) != sizeof(void*) || sizeof(array[0]) <= sizeof(void*))))

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// Window
GLFWwindow* window;

// Vertex and index objects
GLuint vertex_buffer_object;
GLuint index_buffer_object;
GLuint vertex_array_object;

// Shader
Shader* shader;

// Time
float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

void create_geometry(std::vector<Triangle>& triangles, std::vector<Line>& lines) {
    triangles.push_back((Triangle){.a_pos = {0.0f, 0.0f, -1.0f},
                                   .b_pos = {1.0f, 0.0f, -1.0f},
                                   .c_pos = {0.0f, 1.0f, -1.0f},
                                   .a_col = YELLOW_COLOR,
                                   .b_col = YELLOW_COLOR,
                                   .c_col = YELLOW_COLOR});
    triangles.push_back((Triangle){.a_pos = {0.0f, 0.0f, -2.0f},
                                   .b_pos = {1.0f, 0.0f, -2.0f},
                                   .c_pos = {0.0f, 1.0f, -2.0f},
                                   .a_col = YELLOW_COLOR,
                                   .b_col = YELLOW_COLOR,
                                   .c_col = YELLOW_COLOR});
    lines.push_back((Line){.a_pos = {0.4f, 0.4f, 0.0f},
                           .b_pos = {0.4f, 0.4f, -1.5f},
                           .a_col = GREEN_COLOR,
                           .b_col = GREEN_COLOR});
    lines.push_back((Line){.a_pos = {0.0f, 2.0f, -3.0f},
                           .b_pos = {2.0f, 0.0f, -1.0f},
                           .a_col = GREEN_COLOR,
                           .b_col = GREEN_COLOR});
}

void create_vertex_data(std::vector<Triangle>& triangles, std::vector<Line>& lines,
                        std::vector<float>& vertex_data, std::vector<GLshort>& index_data) {
    size_t index_count = 0;

    // Add triangle vertex positions
    for (auto triangle : triangles) {
        vertex_data.push_back(triangle.a_pos.x);
        vertex_data.push_back(triangle.a_pos.y);
        vertex_data.push_back(triangle.a_pos.z);
        index_data.push_back(index_count++);
        vertex_data.push_back(triangle.b_pos.x);
        vertex_data.push_back(triangle.b_pos.y);
        vertex_data.push_back(triangle.b_pos.z);
        index_data.push_back(index_count++);
        vertex_data.push_back(triangle.c_pos.x);
        vertex_data.push_back(triangle.c_pos.y);
        vertex_data.push_back(triangle.c_pos.z);
        index_data.push_back(index_count++);
    }

    // Add line vertex positions
    for (auto line : lines) {
        vertex_data.push_back(line.a_pos.x);
        vertex_data.push_back(line.a_pos.y);
        vertex_data.push_back(line.a_pos.z);
        index_data.push_back(index_count++);
        vertex_data.push_back(line.b_pos.x);
        vertex_data.push_back(line.b_pos.y);
        vertex_data.push_back(line.b_pos.z);
        index_data.push_back(index_count++);
    }

    // Add triangle vertex colors
    for (auto triangle : triangles) {
        vertex_data.push_back(triangle.a_col.r);
        vertex_data.push_back(triangle.a_col.g);
        vertex_data.push_back(triangle.a_col.b);
        vertex_data.push_back(triangle.b_col.r);
        vertex_data.push_back(triangle.b_col.g);
        vertex_data.push_back(triangle.b_col.b);
        vertex_data.push_back(triangle.c_col.r);
        vertex_data.push_back(triangle.c_col.g);
        vertex_data.push_back(triangle.c_col.b);
    }

    // Add line vertex colors
    for (auto line : lines) {
        vertex_data.push_back(line.a_col.r);
        vertex_data.push_back(line.a_col.g);
        vertex_data.push_back(line.a_col.b);
        vertex_data.push_back(line.b_col.r);
        vertex_data.push_back(line.b_col.g);
        vertex_data.push_back(line.b_col.b);
    }
}

bool intersects(const Line& line, const Triangle& triangle) {
    // We will find the point at which the ray intersects the plane defined by the triangle and then
    // check if that point is within the triangle.

    // To find the point at which the ray intersects the plane we will use the fact that the normal
    // of the plane and a vector within the plane would be orthogonal (so their cross product would
    // be 0). We will take the vector on the plane to be defined by one of the points of the
    // triangle and the point at which the ray intersects the plane.
    glm::vec3 n = glm::normalize(
        glm::cross((triangle.b_pos - triangle.a_pos), (triangle.c_pos - triangle.b_pos)));

    glm::vec3 p_tri = triangle.a_pos;

    // We can define the ray with the equation a + d*t = p where p is a point in the ray
    // a would be the start of the line, d the direction and p the point of intersection
    glm::vec3 a = line.a_pos;
    glm::vec3 b = line.b_pos;
    glm::vec3 l = glm::normalize(line.b_pos - line.a_pos);

    // If you put the ray equation inside the cross product mentioned above you can solve for t
    float l_dot_n = glm::dot(l, n);
    if (l_dot_n < TINY_NUMBER && l_dot_n > -TINY_NUMBER) {
        return false;
    }
    float t = glm::dot(p_tri - a, n) / l_dot_n;

    // Now we have the point in the plane
    glm::vec3 p_ray = a + (t * l);

    // If the point is not on the line then the reay doesn't intersect the plane
    if (glm::length(b - a) - glm::length(p_ray - a) - glm::length(b - p_ray) < TINY_NUMBER ||
        glm::length(b - a) - glm::length(p_ray - a) - glm::length(b - p_ray) > -TINY_NUMBER) {
        return false;
    }

    // Check if the point on the plane is inside the triangle by checking if it's to the left of
    // each side
    if (is_left(glm::vec2(triangle.a_pos), glm::vec2(triangle.b_pos), glm::vec2(p_ray)) &&
        is_left(glm::vec2(triangle.b_pos), glm::vec2(triangle.c_pos), glm::vec2(p_ray)) &&
        is_left(glm::vec2(triangle.c_pos), glm::vec2(triangle.a_pos), glm::vec2(p_ray))) {
        return true;
    }

    return false;
}

void mark_intersections(std::vector<Triangle>& triangles, std::vector<Line>& lines) {
    for (auto& line : lines) {
        for (auto& triangle : triangles) {
            if (intersects(line, triangle)) {
                line.a_col = glm::vec3(RED_COLOR);
                line.b_col = glm::vec3(RED_COLOR);
                triangle.a_col = glm::vec3(ORANGE_COLOR);
                triangle.b_col = glm::vec3(ORANGE_COLOR);
                triangle.c_col = glm::vec3(ORANGE_COLOR);
            }
        }
    }
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

void init_vertices(std::vector<float>& vertex_data_in, std::vector<GLshort>& index_data_in,
                   size_t triangle_count, size_t line_count) {
    // Generate c style arrays
    float vertex_data[vertex_data_in.size()];
    for (size_t i = 0; i < vertex_data_in.size(); i++) {
        vertex_data[i] = vertex_data_in[i];
    }

    GLshort index_data[index_data_in.size()];
    for (size_t i = 0; i < index_data_in.size(); i++) {
        index_data[i] = index_data_in[i];
    }

    // Variables
    int vertex_count = (triangle_count * TRI_VERTEX_COUNT) + (line_count * LINE_VERTEX_COUNT);
    int line_index_offset = triangle_count;

    // Bind and generate buffers
    glGenBuffers(1, &vertex_buffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &index_buffer_object);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_object);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_data), index_data, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Vertex array object
    glGenVertexArrays(1, &vertex_array_object);
    glBindVertexArray(vertex_array_object);

    size_t color_data_offset = sizeof(float) * POS_ELEM_COUNT * vertex_count;
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0,              // attribute 0 in shader
                          POS_ELEM_COUNT, // size
                          GL_FLOAT,       // type
                          GL_FALSE,       // normalized?
                          0,              // stride
                          (void*)0        // array buffer offset
    );
    glVertexAttribPointer(1,                       // atrtribute 1 in shader
                          COL_ELEM_COUNT,          // size
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

void draw(size_t triangle_count, size_t line_count) {
    uint triangle_vertex_count = triangle_count * TRI_VERTEX_COUNT;
    uint line_vertex_count = line_count * LINE_VERTEX_COUNT;
    uint line_index_offset = triangle_count * TRI_VERTEX_COUNT;

    glBindVertexArray(vertex_array_object);

    // Draw triangles
    glDrawElements(GL_TRIANGLES, triangle_vertex_count, GL_UNSIGNED_SHORT, 0);

    // Draw lines
    glDrawElementsBaseVertex(GL_LINES, line_vertex_count, GL_UNSIGNED_SHORT, 0, line_index_offset);
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
    init_program();
    init_shaders();

    std::vector<Triangle> triangles;
    std::vector<Line> lines;
    std::vector<float> vertex_data;
    std::vector<GLshort> index_data;
    create_geometry(triangles, lines);
    mark_intersections(triangles, lines);
    create_vertex_data(triangles, lines, vertex_data, index_data);
    init_vertices(vertex_data, index_data, triangles.size(), lines.size());

    // render loop
    while (!glfwWindowShouldClose(window)) {
        // update time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // render
        glClearColor(CLEAR_COLOR, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render
        view_projection_model();
        draw(triangles.size(), lines.size());

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
