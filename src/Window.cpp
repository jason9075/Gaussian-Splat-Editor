#include <OPPCH.h>
#include <SDL_video.h>

bool initWindow(SDL_Window **window, SDL_GLContext *context, const char *title, int &width, int &height) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
    return false;
  }
#ifdef __APPLE__

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#else
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  SDL_DisplayMode current;
  if (SDL_GetCurrentDisplayMode(0, &current) != 0) {
    std::cerr << "Could not get display mode! SDL_Error: " << SDL_GetError() << std::endl;
  } else {
    width = current.w;
    height = current.h;
    // hard code the window size
    width = 1024;
    height = 768;
  }

  *window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height,
                             SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
  if (*window == nullptr) {
    std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
    return false;
  }

  *context = SDL_GL_CreateContext(*window);
  if (*context == nullptr) {
    std::cerr << "OpenGL context could not be created! SDL_Error: " << SDL_GetError() << std::endl;
    return false;
  }

  if (SDL_GL_SetSwapInterval(1) < 0) {
    std::cerr << "Warning: Unable to set VSync! SDL_Error: " << SDL_GetError() << std::endl;
  }

  return true;
}

void close(SDL_Window *window, SDL_GLContext context) {
  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
