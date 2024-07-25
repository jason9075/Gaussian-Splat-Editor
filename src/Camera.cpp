#include "Camera.h"

#include <OPPCH.h>

Camera::Camera(int width, int height, glm::vec3 position, glm::vec3 orientation) {
  this->width = width;
  this->height = height;
  this->position = position;
  this->orientation = orientation;
  this->up = glm::vec3(0.0f, 1.0f, 0.0f);
  this->viewMatrix = glm::lookAt(position, position + orientation, up);
  this->projMatrix = glm::perspective(glm::radians(fov), (float)width / height, nearPlane, farPlane);
}

void Camera::moveForward() { position += movementSpeed * orientation; }

void Camera::moveBackward() { position -= movementSpeed * orientation; }

void Camera::moveLeft() { position -= movementSpeed * glm::normalize(glm::cross(orientation, up)); }

void Camera::moveRight() { position += movementSpeed * glm::normalize(glm::cross(orientation, up)); }

void Camera::moveUp() { position += movementSpeed * up; }

void Camera::moveDown() { position -= movementSpeed * up; }

void Camera::moveFaster() { movementSpeed = 0.5; }

void Camera::moveSlower() { movementSpeed = 0.1; }

void Camera::lookAroundStart(float offsetX, float offsetY) {
  // hide the cursor
  SDL_ShowCursor(SDL_DISABLE);

  if (!relativeMouseMode) {
    relativeMouseMode = true;
    SDL_SetRelativeMouseMode(SDL_TRUE);
  }

  // move mouse to the center of the window
  if (firstClick) {
    SDL_WarpMouseInWindow(NULL, width / 2, height / 2);
    firstClick = false;
  }

  // rotX and rotY is the rotation angle in x and y axis
  float rotX = offsetY * mouseSensitivity;
  float rotY = offsetX * mouseSensitivity;

  // look up and down
  glm::vec3 right = glm::normalize(glm::cross(orientation, up));
  glm::vec3 newOri = glm::rotate(orientation, glm::radians(-rotX), right);

  float angle = glm::degrees(glm::angle(newOri, up));
  // restrict angle between 5 and 175 degrees
  if (angle < 175 && angle > 5) {
    orientation = newOri;
  }

  // look left and right
  orientation = glm::rotate(orientation, glm::radians(-rotY), up);

  // move mouse back to the center of the window again
  SDL_WarpMouseInWindow(NULL, width / 2, height / 2);
}

void Camera::lookAroundEnd() {
  SDL_ShowCursor(SDL_ENABLE);
  relativeMouseMode = false;
  SDL_SetRelativeMouseMode(SDL_FALSE);
  firstClick = true;
}

void Camera::setEventListener(CameraEventListener *listener) { this->listener = listener; }

void Camera::moveCamera() {
  if (keyState[MOVE_FORWARD]) {
    moveForward();
  }
  if (keyState[MOVE_BACKWARD]) {
    moveBackward();
  }
  if (keyState[MOVE_LEFT]) {
    moveLeft();
  }
  if (keyState[MOVE_RIGHT]) {
    moveRight();
  }
  if (keyState[MOVE_UP]) {
    moveUp();
  }
  if (keyState[MOVE_DOWN]) {
    moveDown();
  }
  if (keyState[MOVE_FASTER])
    movementSpeed = fastSpeed;
  else
    movementSpeed = normalSpeed;
}

void Camera::handle(SDL_Event &event) {
  // Handle Keyboard input

  if (event.type == SDL_KEYDOWN && listener != nullptr) {
    // only trigger the event once
    if (event.key.repeat == 0) {
      listener->onKeyDown(event.key.keysym.sym);
    }
  } else if (event.type == SDL_KEYUP && listener != nullptr) {
    listener->onKeyUp(event.key.keysym.sym);
  }

  // Handle Mouse input , move the camera orientation when left mouse button is pressed
  if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT && event.type == SDL_MOUSEMOTION)) {
    listener->onMouseLeftPress(event);
  } else if (event.button.button == SDL_BUTTON_LEFT && event.type == SDL_MOUSEBUTTONUP) {
    listener->onMouseLeftRelease();
  }
}

// Update the camera matrix in the shader
// view matrix: camera position, camera orientation, up vector
// projection matrix: field of view, aspect ratio, near plane, far plane
// camera matrix: projection * view
void Camera::update(Shader *shaderProgram) {
  viewMatrix = glm::lookAt(position, position + orientation, up);
  projMatrix = glm::perspective(glm::radians(fov), (float)width / height, nearPlane, farPlane);

  glUniformMatrix4fv(glGetUniformLocation(shaderProgram->ID, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram->ID, "camMatrix"), 1, GL_FALSE,
                     glm::value_ptr(projMatrix * viewMatrix));
}

GhostCameraListener::GhostCameraListener(Camera *camera) : camera(camera) {}
void GhostCameraListener::onKeyDown(SDL_Keycode key) {
  switch (key) {
    case SDLK_w:
      camera->keyState[MOVE_FORWARD] = true;
      break;
    case SDLK_a:
      camera->keyState[MOVE_LEFT] = true;
      break;
    case SDLK_s:
      camera->keyState[MOVE_BACKWARD] = true;
      break;
    case SDLK_d:
      camera->keyState[MOVE_RIGHT] = true;
      break;
    case SDLK_SPACE:
      camera->keyState[MOVE_UP] = true;
      break;
    case SDLK_c:
      camera->keyState[MOVE_DOWN] = true;
      break;
    case SDLK_LSHIFT:  // Increase movement speed
      camera->keyState[MOVE_FASTER] = true;
      break;
  }
}
void GhostCameraListener::onKeyUp(SDL_Keycode key) {
  switch (key) {
    case SDLK_w:
      camera->keyState[MOVE_FORWARD] = false;
      break;
    case SDLK_a:
      camera->keyState[MOVE_LEFT] = false;
      break;
    case SDLK_s:
      camera->keyState[MOVE_BACKWARD] = false;
      break;
    case SDLK_d:
      camera->keyState[MOVE_RIGHT] = false;
      break;
    case SDLK_SPACE:
      camera->keyState[MOVE_UP] = false;
      break;
    case SDLK_c:
      camera->keyState[MOVE_DOWN] = false;
      break;
    case SDLK_LSHIFT:  // Decrease movement speed
      camera->keyState[MOVE_FASTER] = false;
      break;
  }
}

void GhostCameraListener::onMouseLeftPress(SDL_Event &e) { camera->lookAroundStart(e.motion.xrel, e.motion.yrel); }

void GhostCameraListener::onMouseLeftRelease() { camera->lookAroundEnd(); }
