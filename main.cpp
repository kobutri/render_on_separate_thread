#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

void framebuffer_size_callback(GLFWwindow* window, int _width, int height);
void processInput(GLFWwindow* window);
void render();

void setTransform();
void setCamera();

unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;

GLFWwindow* window = nullptr;
std::atomic_bool quit = false;
std::mutex m;
std::mutex m2;
std::mutex m3;
std::condition_variable cond;
unsigned int shaderProgram;
unsigned int VBO, VAO, EBO;

float vertices[] = {
    1.0f, 0.0f, 0.0f, // top right
    1.0f, 1.0f, 0.0f, // bottom right
    0.0f, 1.0f, 0.0f, // bottom left
    0.0f, 0.0f, 0.0f // top left
};

unsigned int indices[] = {
    0,
    1,
    3,
    1,
    2,
    3,
};

extern "C" const char vertex_shader_source[];
extern "C" const size_t vertex_shader_source_len;

extern "C" const char fragment_shader_source[];
extern "C" const size_t fragment_shader_source_len;

int main()
{
    auto start = std::chrono::high_resolution_clock::now();
    glfwInit();

    glfwWindowHint(GLFW_SAMPLES, 8);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(
        SCR_WIDTH, SCR_HEIGHT, "Render Thread Test", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetWindowSizeCallback(window, framebuffer_size_callback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_MULTISAMPLE);

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* shader_source = vertex_shader_source;
    GLint shader_length = vertex_shader_source_len;
    glShaderSource(vertexShader, 1, &shader_source, &shader_length);
    glCompileShader(vertexShader);
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    shader_source = fragment_shader_source;
    shader_length = fragment_shader_source_len;
    glShaderSource(fragmentShader, 1, &shader_source, &shader_length);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    bool first = true;
    m.lock();
    glfwMakeContextCurrent(nullptr);

    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() << "us" << std::endl;
    std::thread t([]() {
        glfwMakeContextCurrent(window);
        double _time = 0;
        size_t count = 0;
        while (!quit) {
            double start = glfwGetTime();
            m2.lock();
            m.lock();
            m2.unlock();
            render();
            glfwSwapBuffers(window);
            cond.notify_all();
            m.unlock();
            _time += glfwGetTime() - start;
            count++;
            if (count % 60 == 0) {
                std::cout << 1000.0 * _time / 60 << "ms" << std::endl;
                _time = 0;
            }
        }
    });
    do {
        if (first) {
            first = !first;
        } else {
            m2.lock();
            m.lock();
            m2.unlock();
        }
        if (glfwWindowShouldClose(window)) {
            quit = true;
        }
        processInput(window);
        std::unique_lock<std::mutex> lock(m3);
        m.unlock();
        cond.wait(lock);
        lock.unlock();
        glfwPollEvents();
    } while (!quit);

    t.join();
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return EXIT_SUCCESS;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        quit = true;
        glfwSetWindowShouldClose(window, true);
    }
}

double _time = 0.0;
size_t count = 0;

void framebuffer_size_callback(GLFWwindow* _window, int _width, int _height)
{
    double start = glfwGetTime();
    m2.lock();
    m.lock();
    m2.unlock();
    SCR_WIDTH = _width;
    SCR_HEIGHT = _height;
    std::unique_lock<std::mutex> lock(m3);
    m.unlock();
    cond.wait(lock);
    lock.unlock();

    _time += glfwGetTime() - start;
    count++;
    if (count % 20 == 0) {
        // std::cout << 1000.0*_time/20 << "ms" << std::endl;
        _time = 0;
    }
}

void render()
{
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    if (viewport[2] != SCR_WIDTH || viewport[3] != SCR_HEIGHT) {
        glfwSwapInterval(0);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    } else {
        glfwSwapInterval(1);
    }

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    setTransform();
    setCamera();

    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void setTransform()
{
    glm::mat4 trans = glm::mat4(1.0f);
    trans = glm::translate(trans, glm::vec3(SCR_WIDTH / 2, SCR_HEIGHT / 2, 0));
    trans = glm::rotate<float>(trans, glfwGetTime(), glm::vec3(0.0, 0.0f, 1.0f));
    trans = glm::translate(trans, glm::vec3(-100, -100, 0));
    trans = glm::scale(trans, glm::vec3(200, 200, 1.0));
    glUseProgram(shaderProgram);
    unsigned int transLoc = glGetUniformLocation(shaderProgram, "transform");
    glUniformMatrix4fv(transLoc, 1, GL_FALSE, glm::value_ptr(trans));
}

void setCamera()
{
    auto camera = glm::ortho<float>(0, SCR_WIDTH, SCR_HEIGHT, 0, -1000.0f, 1000.0f);
    glUseProgram(shaderProgram);
    unsigned int cameraLoc = glGetUniformLocation(shaderProgram, "camera");
    glUniformMatrix4fv(cameraLoc, 1, GL_FALSE, glm::value_ptr(camera));
}