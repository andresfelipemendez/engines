#include <stdio.h>
#include <math.h>
#include <OpenGL/gl3.h>
#include <GLFW/glfw3.h>

#include "engine.h"
#include "../../thirdparty/nuklear.h"
#include "../../thirdparty/nk_glfw_gl3.h"

static GLFWwindow* s_win = NULL;
static struct nk_glfw s_ui; // mini backend state

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
    nk_glfw_new_frame(&s_ui, fbw, fbh);
    if (nk_begin(&s_ui.ctx, "Demo", nk_rect(10,10,220,140), NK_WINDOW_BORDER|NK_WINDOW_TITLE)){
        nk_layout_row_dynamic(&s_ui.ctx, 24, 1);
        nk_label(&s_ui.ctx, "OpenGL 4.1 + Nuklear", NK_TEXT_LEFT);
        nk_property_float(&s_ui.ctx, "angle", 0.0f, &s_angle, 6.28318f, 0.01f, 0.01f);
    }
    nk_end(&s_ui.ctx);
    nk_glfw_render(&s_ui);
}

static void eng_init(void* glfw_window, int width, int height){
    (void)width; (void)height;
    s_win = (GLFWwindow*)glfw_window;
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    make_triangle_program();
    nk_glfw_init(&s_ui, s_win);
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
    nk_glfw_shutdown(&s_ui);
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