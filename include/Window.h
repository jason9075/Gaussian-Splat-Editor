#pragma once

bool initWindow(SDL_Window **window, SDL_GLContext *context, const char *title, int &width, int &height);

void close(SDL_Window *window, SDL_GLContext context);
