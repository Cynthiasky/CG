#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "shader.h"

using namespace std;
#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 800

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void click_callback(GLFWwindow* window, int button, int action, int modes);
void processInput(GLFWwindow* window);

int fac(int n);
double combination(int n, int i);
glm::vec3 standardize(int x, int y);
vector<glm::vec3> BezierGen();

float lastX = WINDOW_WIDTH / 2.0f;
float lastY = WINDOW_HEIGHT / 2.0f;
bool firstMouse = true;
vector<glm::vec3> control_points;
int control_points_num = 0;
bool onClear = false;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {
    glfwInit();
    // OpenGL version & mode setting
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a window & context/viewpoint setting
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "learn opengl", NULL, NULL);
    if (window == NULL) {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, click_callback);

    // glad init
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    Shader shader("basicshader.vs", "basicshder.fs");

    // Setup ImGui bindings
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
  
    unsigned int VAO, VBO;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    shader.use();

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
   
    
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        processInput(window);
        glfwPollEvents();
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            ImGui::Begin("draw a bezier curve");
            ImGui::Checkbox("clear", &onClear);
            ImGui::End();
        }

        if (onClear) {
                control_points_num = 0;
                control_points.clear();
                onClear = false;
        }
        if (control_points_num >= 2) {
            vector<glm::vec3> curve = BezierGen();
            for (size_t i = 0; i < curve.size(); i++) {
                float point[] = { curve[i].x, curve[i].y, curve[i].z};
                glBindVertexArray(VAO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(point), point, GL_STATIC_DRAW);
                    
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0);
                   
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(point), point, GL_STATIC_DRAW);
          
                glPointSize(2.0f);
                glDrawArrays(GL_POINTS, 0, 1);
            } 
        }

        for (size_t i = 0; i < control_points.size(); i++) {
            float point[] = {control_points[i].x, control_points[i].y, control_points[i].z};
            glBindVertexArray(VAO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(point), point, GL_STATIC_DRAW);
           
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(point), point, GL_STATIC_DRAW);
            glPointSize(5.0f);
            glDrawArrays(GL_POINTS, 0, 1);
        }
       

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}

int fac(int n) {
    if (n == 1 || n == 0)
        return 1;
    for (int i = n - 1;i > 1;i--)
        n = n * i;
    return n;
}


double combination(int n, int i) {
    return fac(n) / (fac(i)*fac(n - i));
}

vector<glm::vec3> BezierGen() {

    vector<glm::vec3> result;   
    for (float t = 0; t <= 1; t += 0.001) {
        glm::vec3 temp = glm::vec3(0.0f, 0.0f, 0.0f);
        for (int i = 0; i < control_points_num; i++) {
            temp.x += combination(control_points_num-1, i) * pow((1-t), control_points_num-1-i) * pow(t, i) * control_points[i].x;
            temp.y += combination(control_points_num-1, i) * pow((1-t), control_points_num-1-i) * pow(t, i) * control_points[i].y;
        }    
        result.push_back(temp);
    }
    return result;
}

void click_callback(GLFWwindow* window, int button, int action, int mods) {
    glm::vec3 clickPos = standardize(lastX, lastY);
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        if (control_points_num >= 1) {
            control_points.pop_back();
            control_points_num--;
        }      
    }     
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        control_points.push_back(standardize(lastX, lastY));
        control_points_num++;
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    lastX = xpos;
    lastY = ypos;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow * window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

glm::vec3 standardize(int x, int y) {
    glm::vec3 result = glm::vec3((float(x) / float(WINDOW_WIDTH)*2.0) - 1, -((float(y) / float(WINDOW_HEIGHT) * 2) - 1), 0.0f);
    return result;
}
