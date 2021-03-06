/*********************************************************************
 * renderer.c                                                        *
 *                                                                   *
 * Copyright (c) 2022 Dmytro Zykov                                   *
 *                                                                   *
 * This file is a part of the vally project, and may only be used,   *
 * modified and distributed under the terms of the MIT License,      *
 * LICENSE.md. By continuing to use, modify and distribute this file *
 * you inidicate that you have read the license and accept it fully. *
 *********************************************************************/

#include <vally/renderer/renderer.h>

#include <glad/glad.h>
#include <cvec.h>

#include <vally/core/event.h>
#include <vally/core/logger.h>
#include <vally/core/window.h>
#include <vally/renderer/texture.h>
#include <vally/renderer/shader.h>
#include <vally/renderer/camera.h>
#include <vally/renderer/projection.h>
#include <vally/renderer/buffer.h>

typedef struct {
  shader *shader_;
  buffer *buffer_;

  texture *tex_white;
  texture *textures[RENDERER_MAX_TEXTURES];
  u32 sampler[RENDERER_MAX_TEXTURES];
  u32 tex_slot;

  cvec vertices;

  u32 index_count;
} renderer_state;

static b8 initialized = FALSE;
static renderer_state state;

static b8 renderer_update_projection(u16 code, void *sender, void *listener, event_context context) {
  shader_send_mat4(state.shader_, "u_proj", camera_update_projection(context.data.f32[0], context.data.f32[1]));
  return TRUE;
}

static b8 renderer_update_view(u16 code, void *sender, void *listener, event_context context) {
  shader_send_mat4(state.shader_, "u_view", camera_update_view());
  return TRUE;
}

b8 renderer_init() {
  if (initialized) {
    return FALSE;
  }

  state.vertices = cvec_create(vertex);

  state.shader_ = shader_create("../assets/shaders/vertex.glsl", "../assets/shaders/fragment.glsl");
  state.buffer_ = buffer_create();

  for (i32 i = 0; i < RENDERER_MAX_TEXTURES; i++) {
    state.sampler[i] = i;
  }

  state.tex_white = texture_white_create();
  state.textures[0] = state.tex_white;

  state.tex_slot = 1;
  state.index_count = 0;

  camera_init();

  shader_send_int_array(state.shader_, "u_image", RENDERER_MAX_TEXTURES, state.sampler);
  shader_send_mat4(state.shader_, "u_proj", camera_update_projection(window_get_width(), window_get_height()));
  shader_send_mat4(state.shader_, "u_view", camera_update_view());

  event_subscribe(EVENT_CODE_CAMERA_MOVED, &state, renderer_update_view);
  event_subscribe(EVENT_CODE_WINDOW_RESIZED, &state, renderer_update_projection);

  initialized = TRUE;
  LOG_TRACE("Renderer system initialized");

  return TRUE;
}

void renderer_begin_batch() {
  if (!initialized) {
    return;
  }

  state.vertices = cvec_create(vertex);
}

void renderer_end_batch() {
  if (!initialized) {
    return;
  }

  buffer_set_data(state.buffer_, cvec_size(state.vertices) * sizeof(vertex), cvec_raw(state.vertices, vertex));
}

void renderer_flush() {
  if (!initialized) {
    return;
  }

  for (i32 i = 0; i < state.tex_slot; i++) {
    texture_bind(i, state.textures[i]);
  }

  shader_attach(state.shader_);
  buffer_bind(state.buffer_);

  glDrawElements(GL_TRIANGLES, state.index_count, GL_UNSIGNED_INT, NULL);

  state.index_count = 0;
  state.tex_slot = 1;
}

void renderer_terminate() {
  if (!initialized) {
    return;
  }

  cvec_destroy(state.vertices);

  initialized = FALSE;

  LOG_TRACE("Renderer system terminated");
}

void renderer_clear_screen() {
  if (!initialized) {
    return;
  }

  // clear screen
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // enable depth test
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_ALWAYS);

  // enable blending
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void renderer_draw(spriterenderer *sprite) {
  if (!initialized) {
    return;
  }

  if (state.index_count >= RENDERER_MAX_INDEX || state.tex_slot > RENDERER_MAX_TEXTURES - 1) {
    renderer_end_batch();
    renderer_flush();
    renderer_begin_batch();
  }

  texture *tex = sprite->texture;
  transform *transform = ecs_transform_get(sprite->entity);
  vec4s color = GLMS_VEC4_ONE;

  f32 tex_id = 0.0f;
  for (i32 i = 1; i < state.tex_slot; i++) {
    if (state.textures[i] == tex) {
      tex_id = (float)i;
      break;
    }
  }

  if (tex_id == 0.0f) {
    tex_id = (float)state.tex_slot;
    state.textures[state.tex_slot] = tex;
    state.tex_slot++;
  }

  vertex ver;
  ver.pos[0] = transform->position.x;
  ver.pos[1] = transform->position.y;
  ver.color[0] = color.x;
  ver.color[1] = color.y;
  ver.color[2] = color.z;
  ver.color[3] = color.w;
  ver.uv[0] = sprite->uvs.uv[0].x;
  ver.uv[1] = sprite->uvs.uv[0].y;
  ver.tex_id = tex_id;
  cvec_push_back(state.vertices, ver);

  ver.pos[0] = transform->position.x + (sprite->size.x * transform->scale.x);
  ver.pos[1] = transform->position.y;
  ver.color[0] = color.x;
  ver.color[1] = color.y;
  ver.color[2] = color.z;
  ver.color[3] = color.w;
  ver.uv[0] = sprite->uvs.uv[1].x;
  ver.uv[1] = sprite->uvs.uv[1].y;
  ver.tex_id = tex_id;
  cvec_push_back(state.vertices, ver);

  ver.pos[0] = transform->position.x + (sprite->size.x * transform->scale.x);
  ver.pos[1] = transform->position.y + (sprite->size.y * transform->scale.y);
  ver.color[0] = color.x;
  ver.color[1] = color.y;
  ver.color[2] = color.z;
  ver.color[3] = color.w;
  ver.uv[0] = sprite->uvs.uv[2].x;
  ver.uv[1] = sprite->uvs.uv[2].y;
  ver.tex_id = tex_id;
  cvec_push_back(state.vertices, ver);

  ver.pos[0] = transform->position.x;
  ver.pos[1] = transform->position.y + (sprite->size.y * transform->scale.y);
  ver.color[0] = color.x;
  ver.color[1] = color.y;
  ver.color[2] = color.z;
  ver.color[3] = color.w;
  ver.uv[0] = sprite->uvs.uv[3].x;
  ver.uv[1] = sprite->uvs.uv[3].y;
  ver.tex_id = tex_id;
  cvec_push_back(state.vertices, ver);

  state.index_count += 6;
}