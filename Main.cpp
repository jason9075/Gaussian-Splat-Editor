#include <OPPCH.h>

#include "App.h"
#include "GUI.h"
#include "Window.h"

// Global variables
const char *WINDOW_TITLE = "OpenGL Demo";

int main(int argc, char *args[]) {
  // init SDL
  SDL_Window *window = nullptr;
  SDL_GLContext context;
  // default screen size
  int *screen_width = new int(1024);
  int *screen_height = new int(768);

  if (!initWindow(&window, &context, WINDOW_TITLE, *screen_width, *screen_height)) {
    std::cerr << "Failed to initialize!" << std::endl;
    exit(1);
  }

  // init glew
  glewExperimental = GL_TRUE;
  GLenum glewError = glewInit();
  if (glewError != GLEW_OK) {
    std::cerr << "Error initializing GLEW! " << glewGetErrorString(glewError) << std::endl;
    exit(1);
  }
  // SDL_GL_SetSwapInterval(0);  // Disable VSync

  App *app = new App(*screen_width, *screen_height);
  GUI gui(window, context, app);

  // main func
  bool quit = false;
  SDL_Event e;

  while (!quit) {
    while (SDL_PollEvent(&e) != 0) {
      if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_q)) {
        quit = true;
      }
      if (!gui.isMouseOverGUI()) app->OnEvent(e);
      ImGui_ImplSDL2_ProcessEvent(&e);  // let imgui interact with SDL
    }

    app->OnRender();
    gui.draw();

    SDL_GL_SwapWindow(window);
  }

  gui.shutdown();

  close(window, context);

  return 0;
}
