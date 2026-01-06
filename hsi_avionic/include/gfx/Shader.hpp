#pragma once
#include <glad/glad.h>

class Shader {
public:
  Shader() = default;
  ~Shader();

  bool build(const char* vs_src, const char* fs_src);
  void use() const;
  GLuint id() const { return program_id_; }

private:
  GLuint program_id_ = 0;
};
