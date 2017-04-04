// Local Headers
#include "glitter.hpp"

// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

using glm::vec3;
using glm::vec4;
using glm::mat4;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>

// Standard Headers
#include <cstdio>
#include <cstdlib>

#include "loadFile.h"

#include <string>
using std::string;

#include <vector>
using std::vector;

#include <memory>
using std::shared_ptr;
using std::make_shared;

#include <chrono>

#include <math.h> // fmod

void checkShaderError(GLuint shader) {
  GLint status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

  if (status != GL_TRUE) {
    char buffer[512];
    glGetShaderInfoLog(shader, 512, NULL, buffer);
    Exception e(buffer);
    throw e;
  }
}

GLuint loadShader(string filename, GLenum type) {
  GLuint shader = glCreateShader(type);

  string source(loadFile(filename));
  const char * cstr = source.c_str();

  glShaderSource(shader, 1, &cstr, NULL);

  glCompileShader(shader);
  checkShaderError(shader);

  return shader;
}

GLuint loadVertexShader(string filename) {
  return loadShader(filename, GL_VERTEX_SHADER);
}

GLuint loadFragmentShader(string filename) {
  return loadShader(filename, GL_FRAGMENT_SHADER);
}

void box(vector<vec3> & vertices, float x0, float x1, float y0, float y1) {
  vec3 tl(x0, y1, 0.0f);
  vec3 bl(x0, y0, 0.0f);
  vec3 tr(x1, y1, 0.0f);
  vec3 br(x1, y0, 0.0f);

  vertices.push_back(tl);
  vertices.push_back(bl);
  vertices.push_back(tr);

  vertices.push_back(tr);
  vertices.push_back(bl);
  vertices.push_back(br);
}

template<typename T>
class Buffer {
public:
  Buffer(shared_ptr<vector<T>> data_) : data(data_) {
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(T) * data->size(),
        glm::value_ptr((*data)[0]),
        GL_STATIC_DRAW);
  }
  void bind() {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
  }
  shared_ptr<vector<T>> data;
  GLuint vbo;
};

class VAO {
public:
  VAO() {
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
  }
  void bind() {
    glBindVertexArray(m_vao);
  }
protected:
  GLuint m_vao;
};

class BoxObject {
public:
  BoxObject(float left, float right, float bottom, float top) {
    m_vao.bind();

    // shader setup
    GLuint m_shader = glCreateProgram();
    glAttachShader(m_shader, loadVertexShader("./Glitter/Sources/vert.glsl"));
    glAttachShader(m_shader, loadFragmentShader("./Glitter/Sources/frag.glsl"));
    glBindFragDataLocation(m_shader, 0, "outColor");
    glLinkProgram(m_shader);
    glUseProgram(m_shader);

    m_vertices = make_shared<vector<vec3>>();
    box(*m_vertices, left, right, bottom, top);

    Buffer<vec3> vertexVBO(m_vertices);
    vertexVBO.bind();
    m_positionAttribute = glGetAttribLocation(m_shader, "position");
    glVertexAttribPointer(m_positionAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(m_positionAttribute);

    // make all vertices grey
    m_color = make_shared<vector<vec4>>();
    std::transform(
        m_vertices->begin(),
        m_vertices->end(),
        std::back_inserter(*m_color),
        [] (const vec3 &) {
            return vec4(0.8, 0.8, 0.8, 1.0);
        }
    );

    Buffer<vec4> colorVBO(m_color);
    colorVBO.bind();
    m_colorAttribute = glGetAttribLocation(m_shader, "color");
    glVertexAttribPointer(m_colorAttribute, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(m_colorAttribute);

    m_modelUniform = glGetUniformLocation(m_shader, "model");
    setPosition(mat4());

    m_viewUniform = glGetUniformLocation(m_shader, "view");
    m_projectionUniform = glGetUniformLocation(m_shader, "proj");
  }
  void render(mat4 & view, mat4 & projection) {
    m_vao.bind();

    glUniformMatrix4fv(m_viewUniform, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(m_projectionUniform, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(m_modelUniform, 1, GL_FALSE, glm::value_ptr(m_position));

    //glDrawArrays(GL_POINTS, 0, m_vertices->size());
    glDrawArrays(GL_TRIANGLES, 0, m_vertices->size());

    glBindVertexArray(0);
  }
  void setPosition(mat4 position) {
    m_position = position;
  }
  const mat4 & getPosition() const {
    return m_position;
  }
  mat4 & getPosition() {
    return m_position;
  }
protected:
  VAO m_vao;
  shared_ptr<vector<vec3>> m_vertices;
  shared_ptr<vector<vec4>> m_color;
  shared_ptr<vector<vec3>> m_normals;
  GLuint m_shader;
  mat4 m_position;

  //shared_ptr<Uniform<double>> m_uniTime;

  GLint m_positionAttribute;
  GLint m_colorAttribute;
  GLint m_modelUniform;
  GLint m_viewUniform;
  GLint m_projectionUniform;
};

int main(int argc, char * argv[]) {

    // Load GLFW and Create a Window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    auto mWindow = glfwCreateWindow(mWidth, mHeight, "OpenGL", nullptr, nullptr);

    // Check for Valid Context
    if (mWindow == nullptr) {
        fprintf(stderr, "Failed to Create OpenGL Context");
        return EXIT_FAILURE;
    }

    // set the camera position
    mat4 view = glm::lookAt(vec3(0,0,1), vec3(0, 0, 0), vec3(0, 1, 0));

    // set up screen coordinates so that (0,0) is center, coordinates are proportional
    // to pixels, and up is +Y
    mat4 projection = glm::ortho(
        -static_cast<float>(mWidth)/2,
        static_cast<float>(mWidth)/2,
        static_cast<float>(mHeight)/2,
        -static_cast<float>(mHeight)/2,
        -1.0f, 1.0f
    );

    // Create Context and Load OpenGL Functions
    glfwMakeContextCurrent(mWindow);
    gladLoadGL();
    fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));

    //float angle = 0.0f;
    BoxObject object(-5.0f, 5.0f, -10.0f, 100.0f);

    auto timeStart = std::chrono::high_resolution_clock::now();
    double timePrev = 0.0;

    // Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false) {
        if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(mWindow, true);

        // Background Fill Color
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // get current time
        auto now = std::chrono::high_resolution_clock::now();
        double time = std::chrono::duration_cast<std::chrono::duration<float>>(now-timeStart).count();

        // time difference in seconds since last frame
        //double delta = time - timePrev;

        mat4 r = glm::rotate(
            mat4(1.0),
            static_cast<float>(time),
            vec3(0.0f,0.0f,1.0f)
        );
        mat4 p = glm::translate(mat4(1.0), vec3(200, 20, 0));

        object.setPosition(p * r);
        object.render(view, projection);

        timePrev = time;

        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);
        glfwPollEvents();
    }
    glfwTerminate();
    return EXIT_SUCCESS;
}
