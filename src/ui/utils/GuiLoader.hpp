//
// Created by 86137 on 2023/12/17.
//

#ifndef ADAPTABLE_UPLOADER_GUILOADER_HPP
#define ADAPTABLE_UPLOADER_GUILOADER_HPP

#include <iostream>
#include <functional>
#include <vector>

#include "glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

using std::vector;

using Update_Function = std::function<void()>;

class Gui_Loader {
public:
    void init() {
        this->opengl_loader();
        this->imgui_loader();
    }

    void add_update_function(const Update_Function& update_function) {
        this->update_functions.push_back(update_function);
    }

    void run() {
        while(!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            for (const Update_Function& function: this->update_functions) {
                function();
            }

            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
            glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                         clear_color.z * clear_color.w, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
        }
    }

    void close() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();
    }

private:
    const char* glsl_version = "#version 330";
    GLFWwindow* window;

    vector<Update_Function> update_functions;

    static void glfw_error_callback(int error, const char *description) {
        std::cerr << "Glfw Error " << error << ": " << description << std::endl;
    }

    void opengl_loader() {
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
            exit(EXIT_FAILURE);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

        window = glfwCreateWindow(800, 600, "Scheduler", nullptr, nullptr);
        if (window == nullptr)
            exit(EXIT_FAILURE);
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);
    }

    void imgui_loader() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void) io;

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(this->glsl_version);
    }
};


#endif //ADAPTABLE_UPLOADER_GUILOADER_HPP
