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
#include <math.h>

#define W_WIDTH 720
#define W_HEIGHT 480

int w_width = W_WIDTH;
int w_height = W_HEIGHT;
float zoom = 100.0f;
float rotation = 0.0f;
double mouse_x;
double mouse_y;
bool mouse_left_down = false;
bool mouse_right_down = false;
bool should_close = false;
GLFWwindow *window;
GLuint vao, vbo;

void window_close_callback(GLFWwindow *w) {
    should_close = true;
}

void error_callback(int error, const char *description) {
    printf("Error: %s\n", description);
}

void cursor_position_callback(GLFWwindow *w, double x_pos, double y_pos) {
    mouse_x = x_pos;
    mouse_y = y_pos;
}

void keyboard_event(GLFWwindow *w, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        should_close = true;
    }
}

void scroll_callback(GLFWwindow *w, double x_offset, double y_offset) {
    if (y_offset == -1.0f) {
        zoom += 5.0f;
    } else {
        zoom -= 5.0f;
    }
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

void set_aspect(int width, int height, GLint mvp_uniform) {
    float aspect = (float) width / (float) height;
    glViewport(0, 0, width, height);
    mat4x4 projection;
    mat4x4_perspective(projection, 45.0f, aspect, 0.1f, 1000.0f);
    mat4x4 view;
    mat4x4_look_at(view, (vec3) {0.0f, 0.0f, zoom}, (vec3) {0.0f, 0.0f, 0.0f}, (vec3) {0.0f, 1.0f, 0.0f});

    mat4x4 model;
    mat4x4_identity(model);
    mat4x4_rotate_Y(model, model, rotation);
    //mat4x4_rotate_Z(model, model, rotation);
    mat4x4 mvp;
    mat4x4_mul(mvp, projection, view);
    mat4x4_mul(mvp, mvp, model);
    glUniformMatrix4fv(mvp_uniform, 1, GL_FALSE, (const GLfloat *) mvp);
}

void resize_callback(GLFWwindow *w, int width, int height) {
    w_width = width;
    w_height = height;
}

void render_mesh(const struct aiMesh *mesh) {
    glBindVertexArray(vao);

    // Create and bind the index buffer
    GLuint ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    // Calculate the size of the buffer data
    size_t vertexDataSize = sizeof(float) * 3 * mesh->mNumVertices;
    size_t indexDataSize = sizeof(unsigned int) * 3 * mesh->mNumFaces;

    // Allocate memory for the buffer data
    float *vertexData = (float *) malloc(vertexDataSize);
    unsigned int *indexData = (unsigned int *) malloc(indexDataSize);
    if (vertexData == NULL || indexData == NULL) {
        printf("Failed to allocate memory for vertex or index data\n");
        return;
    }

    // Copy vertex data into the buffer
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        vertexData[i * 3] = mesh->mVertices[i].x;
        vertexData[i * 3 + 1] = mesh->mVertices[i].y;
        vertexData[i * 3 + 2] = mesh->mVertices[i].z;
    }

    // Copy index data into the buffer
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        const struct aiFace *face = &mesh->mFaces[i];
        if (face->mNumIndices != 3) {
            printf("Only triangles are supported\n");
            continue;
        }

        indexData[i * 3] = face->mIndices[0];
        indexData[i * 3 + 1] = face->mIndices[1];
        indexData[i * 3 + 2] = face->mIndices[2];
    }

    // Set the vertex buffer data
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexDataSize, vertexData, GL_STATIC_DRAW);

    // Set the index buffer data
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSize, indexData, GL_STATIC_DRAW);

    // Free the allocated memory
    free(vertexData);
    free(indexData);

    // Enable vertex attribute and set the pointer
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Draw the elements using the index buffer
    glDrawElements(GL_TRIANGLES, 3 * mesh->mNumFaces, GL_UNSIGNED_INT, 0);

    // Disable vertex attribute
    glDisableVertexAttribArray(0);

    // Delete the index buffer
    glDeleteBuffers(1, &ibo);
}

void render_scene(const struct aiScene *scene) {
    // Generate VAO and VBO for each mesh
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    // Render each mesh in the scene
    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        const struct aiMesh *mesh = scene->mMeshes[i];
        render_mesh(mesh);
    }

    // Cleanup VAO and VBO after rendering
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

int main() {
    if (!glfwInit()) {
        printf("Could not initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    glfwSetErrorCallback(error_callback);

    window = glfwCreateWindow(w_width, w_height, "sand", NULL, NULL);
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
    glfwSwapInterval(1); // v-sync

    glewExperimental = GL_TRUE;
    glewInit();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // shader
    char *vs = "#version 330 core\n"
               "layout (location = 0) in vec3 in_Position;\n"
               "uniform mat4 mvp;\n"
               "out vec3 vertexColor;\n"
               "void main()\n"
               "{\n"
               "    gl_Position = mvp * vec4(in_Position, 1.0f);\n"
               "    vertexColor = vec3(1.0f, 1.0f, 1.0f); // Default color: white\n"
               "}\n\0";
    char *fs = "#version 330 core\n"
               "out vec4 FragColor;\n"
               "in vec3 vertexColor;\n"
               "void main()\n"
               "{\n"
               "    FragColor = vec4(vertexColor, 1.0f);\n"
               "}\n\0";

    GLuint program = shader_program_create_s(vs, fs);
    shader_program_bind_attribute_location(program, 0, "in_Position");
    shader_program_bind_attribute_location(program, 1, "in_Color");
    shader_program_link(program);
    GLint mvp_uniform = shader_program_get_uniform_location(program, "mvp");

    // vao
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // vbo
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    const struct aiScene *scene = aiImportFile("assets/airboat.obj", aiProcess_Triangulate);
    if (!scene) {
        printf("Failed to load model: %s\n", aiGetErrorString());
        return -1;
    }

    // game loop
    double delta;
    double start_time;
    double previous_time = glfwGetTime();
    int frame_count = 0;

    while (!should_close) {
        start_time = glfwGetTime();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.169f, 0.169f, 0.169f, 1.0f);
        glUseProgram(program);
        set_aspect(w_width, w_height, mvp_uniform);

        if (mouse_left_down) {
        }
        if (mouse_right_down) {
        }

        rotation += 0.003f;

        render_scene(scene);

        glfwSwapBuffers(window);
        glfwPollEvents();
        delta = glfwGetTime() - start_time;
        frame_count++;
        if (start_time - previous_time >= 1.0) {
            printf("frame: %.2f, fps: %d, mouse: %f, %f\n",
                   delta * 1000, frame_count, mouse_x, mouse_y);
            previous_time = start_time;
            frame_count = 0;
        }
    }

    // free memory for scene
    aiReleaseImport(scene);
    glfwTerminate();
    return 0;
}
