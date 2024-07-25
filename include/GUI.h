#pragma once

#include "App.h"

class GUI {
 public:
  App*& app;

  GUI(SDL_Window* window, SDL_GLContext context, App*& app);
  void draw();
  void shutdown();
  bool isMouseOverGUI();
};
