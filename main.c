#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "shader.h"
#include "linmath.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define W_WIDTH 1920
#define W_HEIGHT 1080

int w_width = W_WIDTH;
int w_height = W_HEIGHT;
mat4x4 mvp;
float zoom;
double mouse_x;
double mouse_y;
bool mouse_left_down = false;
bool mouse_right_down = false;
bool should_close = false;

void window_close_callback(GLFWwindow *w) {
    should_close = true;
}

void set_aspect(int width, int height) {
    float aspect = (float) width / (float) height;
    glViewport(0, 0, W_WIDTH, W_HEIGHT);
    gluOrtho2D(0.0f, (float) W_WIDTH, (float) W_HEIGHT, 0.0f);
    mat4x4 m, p;
    mat4x4_identity(m);
    mat4x4_ortho(p, -aspect * zoom, aspect * zoom, zoom, -zoom, 1, -1);
    mat4x4_translate_in_place(p, -aspect * zoom, -zoom, -1);
    mat4x4_mul(mvp, p, m);
}

void resize_callback(GLFWwindow *w, int width, int height) {
    w_width = width;
    w_height = height;
    set_aspect(w_width, w_height);
}

void error_callback(int error, const char *description) {
    printf("Error: %s\n", description);
}

void cursor_position_callback(GLFWwindow *w, double x_pos,
                              double y_pos) {
    mouse_x = x_pos;
    mouse_y = y_pos;
}

void keyboard_event(GLFWwindow *w, int key, int scancode, int action,
                    int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        should_close = true;
    }
}

void scroll_callback(GLFWwindow *w, double x_offset, double y_offset) {
    //    if (y_offset == -1.0f) {
    //        if (zoom < 100.0f) { zoom += 5.0f; }
    //    } else {
    //        if (zoom > 5.0f) { zoom -= 5.0f; }
    //    }
}

void mouse_button_callback(GLFWwindow *w, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        mouse_right_down = true;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        mouse_right_down = false;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        mouse_left_down = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        mouse_left_down = false;
    }
}

void ffree(void *obj) {
    if (obj != NULL) {
        free(obj);
        obj = NULL;
    }
}

void checkm(void *obj) {
    if (obj == NULL) {
        printf("Could not allocate memory for object\n");
        exit(-1);
    }
}

int main() {

    if (!glfwInit()) {
        printf("Could not initialize GLFW\n");
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwSetErrorCallback(error_callback);

    GLFWwindow *window = glfwCreateWindow(w_width, w_height, "sand", NULL,
                                          NULL);
    if (!window) {
        glfwTerminate();
        printf("Could not create window\n");
        return -1;
    }
    // keyboard
    glfwSetKeyCallback(window, keyboard_event);
    //mouse
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    // window
    glfwSetWindowCloseCallback(window, window_close_callback);
    glfwSetFramebufferSizeCallback(window, resize_callback);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);// v-sync
    glewExperimental = GL_TRUE;
    glewInit();
    glDisable(GL_DEPTH_TEST);
    set_aspect(w_width, w_height);

    const struct aiScene *scene = aiImportFile("assets/airboat.obj", aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene) {
        printf("Failed to load model: %s\n", aiGetErrorString());
        return -1;
    }
    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        const struct aiMesh *mesh = scene->mMeshes[i];
        // Process the mesh data
    }
    aiReleaseImport(scene);

    // game loop
    double delta;
    double start_time;
    double previous_time = glfwGetTime();
    int frame_count = 0;

    while (!should_close) {
        start_time = glfwGetTime();
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.169f, 0.169f, 0.169f, 1.0f);
        set_aspect(w_width, w_height);

        if (mouse_left_down) {
        }
        if (mouse_right_down) {
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
        delta = glfwGetTime() - start_time;
        frame_count++;
        if (start_time - previous_time >= 1.0) {
            printf("frame: %.2f, fps: %d, mouse: %f, %f\n",
                   delta * 1000, frame_count, mouse_x,
                   mouse_y);
            previous_time = start_time;
            frame_count = 0;
        }
    }

    glfwTerminate();
    return 0;
}
