#include <stdio.h>
#include <math.h>
#include <OpenGL/gl3.h>
#include <GLFW/glfw3.h>

#include "engine.h"
#include "../../thirdparty/nuklear/nuklear.h"
#include "../../thirdparty/nuklear/nuklear_glfw_gl4.h"

static GLFWwindow* s_win = NULL;


static GLuint s_prog=0, s_vao=0, s_vbo=0;
static float  s_angle=0.0f;

static GLuint compile(GLenum type, const char* src){
    GLuint s = glCreateShader(type);
    glShaderSource(s,1,&src,NULL);
    glCompileShader(s);
    GLint ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if(!ok){ char log[1024]; GLsizei n=0; glGetShaderInfoLog(s,1024,&n,log); fprintf(stderr,"shader err: %.*s\n",n,log);}    
    return s;
}

static void make_triangle_program(void){
    const char* vs =
        "#version 410 core\n"
        "layout(location=0) in vec3 aPos;\n"
        "uniform float u_angle;\n"
        "void main(){\n"
        "  mat2 R = mat2(cos(u_angle), -sin(u_angle), sin(u_angle), cos(u_angle));\n"
        "  vec2 p = R * aPos.xy;\n"
        "  gl_Position = vec4(p, aPos.z, 1.0);\n"
        "}";
    const char* fs =
        "#version 410 core\n"
        "out vec4 o;\n"
        "void main(){ o = vec4(0.1,0.8,0.4,1.0); }";
    GLuint v = compile(GL_VERTEX_SHADER, vs);
    GLuint f = compile(GL_FRAGMENT_SHADER, fs);
    s_prog = glCreateProgram();
    glAttachShader(s_prog, v); glAttachShader(s_prog, f);
    glLinkProgram(s_prog);
    glDeleteShader(v); glDeleteShader(f);

    float verts[] = {
        0.0f,  0.6f, 0.0f,
       -0.6f, -0.6f, 0.0f,
        0.6f, -0.6f, 0.0f
    };
    glGenVertexArrays(1,&s_vao);
    glGenBuffers(1,&s_vbo);
    glBindVertexArray(s_vao);
    glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glBindVertexArray(0);
}

static void ui_frame(int fbw, int fbh){
    
}

static void eng_init(void* glfw_window, int width, int height){
    (void)width; (void)height;
    s_win = (GLFWwindow*)glfw_window;
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    make_triangle_program();
    // nk_glfw_init(&s_ui, s_win);
}

static void eng_resize(int width, int height){
    glViewport(0,0,width,height);
}

static void eng_update(float dt){
    int fbw, fbh; glfwGetFramebufferSize(s_win, &fbw, &fbh);
    glViewport(0,0,fbw,fbh);
    glClearColor(0.08f,0.08f,0.10f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    s_angle += dt;
    glUseProgram(s_prog);
    GLint ua = glGetUniformLocation(s_prog, "u_angle");
    glUniform1f(ua, s_angle);
    glBindVertexArray(s_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    ui_frame(fbw, fbh);
}

static void eng_shutdown(void){
    
    glDeleteProgram(s_prog);
    glDeleteVertexArrays(1,&s_vao);
    glDeleteBuffers(1,&s_vbo);
    s_prog=s_vao=s_vbo=0;
}

void get_engine_api(struct EngineAPI* out_api){
    out_api->init     = eng_init;
    out_api->resize   = eng_resize;
    out_api->update   = eng_update;
    out_api->shutdown = eng_shutdown;
}