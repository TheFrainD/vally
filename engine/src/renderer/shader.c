#include "shader.h"

#include <glad/glad.h>

#include "resources/text_loader.h"
#include "core/logger.h"

static b8 check_gl_errors() {
  b8 error = FALSE;
  i32 glerr = glGetError();
  while (glerr != GL_NO_ERROR) {
    VALLY_ERROR("OpenGL error: %d", glerr);
    error = TRUE;
    glerr = glGetError();
  }

  return error;
}

static b8 shader_err_log(GLuint handle, GLenum shader_type) {
  b8 error = FALSE;
  i32 len;
  i32 ch;
  char *log;
  glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &len);
  if (len > 0) {
    error = TRUE;
    log = (char *)malloc(len);
    glGetShaderInfoLog(handle, len, &ch, log);
    VALLY_ERROR("%s shader log: %s", shader_type == GL_VERTEX_SHADER ? "Vertex" : "Fragment", log);
    free(log);
  }
  return error;
}

static b8 program_err_log(i32 program) {
  b8 error = FALSE;
  i32 len;
  i32 ch;
  char *log;
  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
  if (len > 0) {
    error = TRUE;
    log = (char *)malloc(len);
    glGetProgramInfoLog(program, len, &ch, log);
    VALLY_ERROR("Shader program log: %s", log);
    free(log);
  }
  return error;
}

static GLuint compile_shader(const char *source, GLenum shader_type) {
  GLuint handle = glCreateShader(shader_type);
  glShaderSource(handle, 1, (const GLchar *const *)&source, 0);

  GLuint compiled;
  glCompileShader(handle);
  check_gl_errors();
  glGetShaderiv(handle, GL_COMPILE_STATUS, &compiled);
  if (!compiled) {
    VALLY_ERROR("%s shader compilation failed!", shader_type == GL_VERTEX_SHADER ? "Vertex" : "Fragment");
    shader_err_log(handle, shader_type);
    VALLY_ERROR("Source code: %s", source);
  }

  return handle;
}

shader shader_create(const char *vertex_path, const char *fragment_path) {
  shader shdr;
  shdr.attached = FALSE;

  const char* vsource = load_text_file(vertex_path);
  const char* fsource = load_text_file(fragment_path);

  GLuint vshader = compile_shader(vsource, GL_VERTEX_SHADER);
  GLuint fshader = compile_shader(fsource, GL_FRAGMENT_SHADER);

  GLuint linked;
  shdr.id = glCreateProgram();
  glAttachShader(shdr.id, vshader);
  glAttachShader(shdr.id, fshader);

  glLinkProgram(shdr.id);
  check_gl_errors();
  glGetProgramiv(shdr.id, GL_LINK_STATUS, &linked);
  if (!linked) {
    VALLY_ERROR("Shader program linking failed!");
    program_err_log(shdr.id);
  }

  glDeleteShader(vshader);
  glDeleteShader(fshader);

  return shdr;
}

void shader_attach(shader *shdr) {
  if (!shdr->attached) {
    glUseProgram(shdr->id);
    shdr->attached = TRUE;
  }
}

void shader_deattach(shader *shdr) {
  glUseProgram(0);
  shdr->attached = FALSE;
}

void shader_destroy(shader *shdr) {
  glDeleteProgram(shdr->id);
}

void shader_send_float(shader *shdr, const char *name, f32 value) {
  shader_attach(shdr);
  glUniform1f(glGetUniformLocation(shdr->id, name), value);
}

void shader_send_float_array(shader *shdr, const char *name, i32 count, const f32 *array) {
  shader_attach(shdr);
  glUniform1fv(glGetUniformLocation(shdr->id, name), count, array);
}

void shader_send_vec2(shader *shdr, const char *name, vec2s vector) {
  shader_attach(shdr);
  glUniform2f(glGetUniformLocation(shdr->id, name), vector.x, vector.y);
}

void shader_send_vec3(shader *shdr, const char *name, vec3s vector) {
  shader_attach(shdr);
  glUniform3f(glGetUniformLocation(shdr->id, name), vector.x, vector.y, vector.z);
}

void shader_send_vec4(shader *shdr, const char *name, vec4s vector) {
  shader_attach(shdr);
  glUniform4f(glGetUniformLocation(shdr->id, name), vector.x, vector.y, vector.z, vector.w);
}

void shader_send_int(shader *shdr, const char *name, int value) {
  shader_attach(shdr);
  glUniform1i(glGetUniformLocation(shdr->id, name), value);
}

void shader_send_int_array(shader *shdr, const char *name, i32 count, const i32 *array) {
  shader_attach(shdr);
  glUniform1iv(glGetUniformLocation(shdr->id, name), count, array);
}

void shader_send_mat4(shader *shdr, const char *name, mat4s matrix) {
  shader_attach(shdr);
  glUniformMatrix4fv(glGetUniformLocation(shdr->id, name), 1, GL_FALSE, (f32*)matrix.raw);
}
