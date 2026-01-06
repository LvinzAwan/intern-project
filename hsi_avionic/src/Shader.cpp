#include "gfx/Shader.hpp"
#include <iostream>

static GLuint compile_shader(GLenum type, const char* src) {
  GLuint s = glCreateShader(type);
  glShaderSource(s, 1, &src, nullptr);
  glCompileShader(s);

  GLint ok = 0;
  glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    char log[1024];
    glGetShaderInfoLog(s, 1024, nullptr, log);
    std::cerr << "Shader compile error:\n" << log << "\n";
    glDeleteShader(s);
    return 0;
  }
  return s;
}

Shader::~Shader() {
  if (program_id_) glDeleteProgram(program_id_);
}

bool Shader::build(const char* vs_src, const char* fs_src) {
  GLuint vs = compile_shader(GL_VERTEX_SHADER, vs_src);
  GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fs_src);
  if (!vs || !fs) return false;

  program_id_ = glCreateProgram();
  glAttachShader(program_id_, vs);
  glAttachShader(program_id_, fs);
  glLinkProgram(program_id_);

  glDeleteShader(vs);
  glDeleteShader(fs);

  GLint ok = 0;
  glGetProgramiv(program_id_, GL_LINK_STATUS, &ok);
  if (!ok) {
    char log[1024];
    glGetProgramInfoLog(program_id_, 1024, nullptr, log);
    std::cerr << "Program link error:\n" << log << "\n";
    glDeleteProgram(program_id_);
    program_id_ = 0;
    return false;
  }
  return true;
}

void Shader::use() const {
  glUseProgram(program_id_);
}
