#include <vally/core/window.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vally/core/logger.h>
#include <vally/core/event.h>

typedef struct {
  GLFWwindow *window;
  i32 width;
  i32 height;
  char *title;
} window_state;

static b8 initialized = FALSE;
static window_state state;

static void window_size_callback(GLFWwindow *window, i32 width, i32 height) {
  state.width = width;
  state.height = height;
  glViewport(0, 0, width, height);
  event_context context;
  context.data.f32[0] = (float)width;
  context.data.f32[1] = (float)height;
  event_publish(EVENT_CODE_WINDOW_RESIZED, &state, context);
}

static void window_error_callback(i32 id, const char *description) {
  VALLY_ERROR("GLFW error: %s", description);
}


b8 window_create(i32 width, i32 height, const char* title) {
  if (initialized) {
    return FALSE;
  }
  state.width = width;
  state.height = height;
  state.title = title;

  glfwSetErrorCallback(window_error_callback);
  if (!glfwInit()) {
    VALLY_FATAL("Could not initialize GLFW!");
    return FALSE;
  }
  VALLY_TRACE("GLFW initialized");

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  state.window = glfwCreateWindow(width, height, title, NULL, NULL);
  if (!state.window) {
    VALLY_FATAL("Could not create window!");
    return FALSE;
  }
  VALLY_TRACE("Window created");
  glfwMakeContextCurrent(state.window);
  glfwSwapInterval(1);

  glfwSetWindowSizeCallback(state.window, window_size_callback);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    VALLY_FATAL("Could not initialize glad!");
    return FALSE;
  }
  VALLY_TRACE("glad initialized");

  glViewport(0, 0, width, height);

  VALLY_INFO("====System Info====");
  VALLY_INFO("Vendor: %s", glGetString(GL_VENDOR));
  VALLY_INFO("Renderer: %s,", glGetString(GL_RENDERER));
  i32 major, minor;
  glGetIntegerv(GL_MAJOR_VERSION, &major);
  glGetIntegerv(GL_MINOR_VERSION, &minor);
  VALLY_INFO("OpenGL version: %d.%d", major, minor);
  VALLY_INFO("===================");


  return TRUE;
}

b8 window_poll_events() {
  glfwPollEvents();
  return !glfwWindowShouldClose(state.window);
}

void window_swap_buffers() {
  glfwSwapBuffers(state.window);
}

void window_terminate() {
  glfwDestroyWindow(state.window);
  glfwTerminate();
  VALLY_TRACE("Window subsystem terminated");
}

void window_set_key_callback(void *callback) {
  glfwSetKeyCallback(state.window, callback);
}

void window_set_mouse_button_callback(void *callback) {
  glfwSetMouseButtonCallback(state.window, callback);
}

void window_set_cursor_position_callback(void *callback) {
  glfwSetCursorPosCallback(state.window, callback);
}

i32 window_get_width() {
  return state.width;
}

i32 window_get_height() {
  return state.height;
}