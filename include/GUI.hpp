#pragma once

#include "App.hpp"

class GUI {
 public:
  App*& app;

  GUI(SDL_Window* window, SDL_GLContext context, App*& app);
  void draw();
  void shutdown();
  bool isMouseOverGUI();
};
