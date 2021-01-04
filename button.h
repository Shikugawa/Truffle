//
// Created by shikugawa on 2020/12/27.
//

#ifndef TRUFFLE_BUTTON_H
#define TRUFFLE_BUTTON_H

#include <SDL2/SDL.h>

#include <cassert>
#include <set>
#include <unordered_map>

#include "common/exception.h"
#include "common/logger.h"
#include "common/stateful_object_manager.h"
#include "message.h"
#include "texture.h"

namespace Truffle {

enum class ButtonState {
  Normal,
  Hovered,
  Pressed,
};

class ButtonCallback {
 public:
  /**
   * ボタンが押された時のコールバック
   */
  virtual void onButtonPressed() = 0;

  /**
   * ボタンが離された時のコールバック
   */
  virtual void onButtonReleased() = 0;

  /**
   * ボタンがホバーされた時のコールバック
   */
  virtual void onMouseHovered() = 0;

  /**
   * ボタンがアンホバーされた時のコールバック
   */
  virtual void onMouseUnhovered() = 0;

  void _onButtonPressed(SDL_Event& ev);
  void _onButtonReleased(SDL_Event& ev);
  void _onMouseHovered(SDL_Event& ev);
  void _onMouseUnhovered(SDL_Event& ev);

  StatefulObjectManager<ImageTexture, ButtonState> state_manager;

 protected:
  bool isMouseHovered(const SDL_Rect& render_rect);
  bool isMouseUnhovered(const SDL_Rect& render_rect);
  bool isPressed(SDL_Event& ev, const SDL_Rect& render_rect);
  bool isReleased(SDL_Event& ev, const SDL_Rect& render_rect);
};

void ButtonCallback::_onButtonPressed(SDL_Event& ev) {
  if (isPressed(ev, state_manager.activeStateObject().renderRect()) &&
      state_manager.activeState() == ButtonState::Hovered) {
    onButtonPressed();
  }
}

void ButtonCallback::_onButtonReleased(SDL_Event& ev) {
  if (isReleased(ev, state_manager.activeStateObject().renderRect()) &&
      state_manager.activeState() == ButtonState::Pressed) {
    onButtonReleased();
  }
}

void ButtonCallback::_onMouseHovered(SDL_Event& ev) {
  if (isMouseHovered(state_manager.activeStateObject().renderRect()) &&
      state_manager.activeState() == ButtonState::Normal) {
    onMouseHovered();
  }
}

void ButtonCallback::_onMouseUnhovered(SDL_Event& ev) {
  if (isMouseUnhovered(state_manager.activeStateObject().renderRect()) &&
      state_manager.activeState() == ButtonState::Hovered) {
    onMouseUnhovered();
  }
}

bool ButtonCallback::isMouseHovered(const SDL_Rect& render_rect) {
  int mouse_x, mouse_y;
  SDL_GetMouseState(&mouse_x, &mouse_y);
  return render_rect.x < mouse_x && mouse_x < render_rect.x + render_rect.w &&
         render_rect.y < mouse_y && mouse_y < render_rect.y + render_rect.h;
}

bool ButtonCallback::isMouseUnhovered(const SDL_Rect& render_rect) {
  return !isMouseHovered(render_rect);
}

bool ButtonCallback::isPressed(SDL_Event& ev, const SDL_Rect& render_rect) {
  if (ev.type == SDL_MOUSEBUTTONDOWN && ev.button.button == SDL_BUTTON_LEFT) {
    return isMouseHovered(render_rect);
  }
  return false;
}

bool ButtonCallback::isReleased(SDL_Event& ev, const SDL_Rect& render_rect) {
  if (ev.type == SDL_MOUSEBUTTONUP && ev.button.button == SDL_BUTTON_LEFT) {
    return isMouseHovered(render_rect);
  }
  return false;
}

/**
 * 基礎的なボタンの機能を提供するクラス
 */
class BasicButtonObject : public TruffleObject, public ButtonCallback {
 public:
  /**
   * ボタンのコンストラクタ
   *
   * @param parent_controller 親コントローラー
   * @param renderer レンダラ
   * @param name 名前
   * @param x x座標
   * @param y y座標
   * @param path_normal 初期状態のテクスチャのパス
   * @param path_hovered ホバー状態のテクスチャのパス
   * @param path_pressed 押下時のテクスチャのパス
   */
  BasicButtonObject(TruffleController& parent_controller,
                    const Renderer& renderer, std::string name, int x, int y,
                    std::string path_normal, std::string path_hovered = "",
                    std::string path_pressed = "");

  // Renderable
  void render() override final;

  // ButtonEventCallback
  virtual void onButtonPressed() override;
  virtual void onButtonReleased() override;
  virtual void onMouseHovered() override;
  virtual void onMouseUnhovered() override;

  /**
   * 同一シーン内のコントローラーにメッセージを送る。メッセージの送信に失敗した場合は例外を送出する。
   * 失敗した場合はfalseを返す。
   * @param dst_controller
   * @param object_name
   * @param msg
   */
  void sendMessage(std::string dst_controller, std::string dst_object,
                   Message& msg);
  void sendMessage(std::string dst_controller, std::string dst_object,
                   Message&& msg);

 protected:
};

BasicButtonObject::BasicButtonObject(TruffleController& parent_controller,
                                     const Renderer& renderer, std::string name,
                                     int x, int y, std::string path_normal,
                                     std::string path_hovered,
                                     std::string path_pressed)
    : TruffleObject(parent_controller, renderer, name) {
  state_manager.setInitStatefulObject(ButtonState::Normal, parent_controller,
                                      renderer_, path_normal, name + "_normal",
                                      x, y);
  // Bind object
  if (!path_hovered.empty()) {
    state_manager.bindStatefulObject(ButtonState::Hovered, parent_controller,
                                     renderer_, path_hovered, name + "_hovered",
                                     x, y);
  }
  if (!path_pressed.empty()) {
    state_manager.bindStatefulObject(ButtonState::Pressed, parent_controller,
                                     renderer_, path_pressed, name + "_pressed",
                                     x, y);
  }
  // define state transition
  state_manager.setStateTransition(ButtonState::Hovered, ButtonState::Pressed);
  state_manager.setStateTransition(ButtonState::Pressed, ButtonState::Hovered);
  state_manager.setStateTransition(ButtonState::Normal, ButtonState::Hovered);
  state_manager.setStateTransition(ButtonState::Hovered, ButtonState::Normal);

  setPoint(state_manager.activeStateObject().renderRect().x,
           state_manager.activeStateObject().renderRect().y);
  setWidth(state_manager.activeStateObject().renderRect().w);
  setHeight(state_manager.activeStateObject().renderRect().h);

  // Register event callbacks
  setEventCallback([this](SDL_Event& e) { this->_onMouseHovered(e); });
  setEventCallback([this](SDL_Event& e) { this->_onMouseUnhovered(e); });
  setEventCallback([this](SDL_Event& e) { this->_onButtonReleased(e); });
  setEventCallback([this](SDL_Event& e) { this->_onButtonPressed(e); });
}

void BasicButtonObject::render() {
  SDL_RenderCopy(
      const_cast<SDL_Renderer*>(renderer_.entity()),
      const_cast<SDL_Texture*>(state_manager.activeStateObject().entity()),
      nullptr /* TODO: introduce clip settings */, &renderRect());
}

void BasicButtonObject::sendMessage(std::string dst_controller,
                                    std::string dst_object, Message& msg) {
  msg.dst_object = dst_object;
  TruffleObject::sendMessage(dst_controller, msg);
}

void BasicButtonObject::sendMessage(std::string dst_controller,
                                    std::string dst_object, Message&& msg) {
  msg.dst_object = dst_object;
  TruffleObject::sendMessage(dst_controller, msg);
}

void BasicButtonObject::onButtonPressed() {
  // log_(LogLevel::INFO, "State changed from Hovered to Pressed");
  state_manager.stateTransition(ButtonState::Pressed);
  setWidth(state_manager.activeStateObject().renderRect().w);
  setHeight(state_manager.activeStateObject().renderRect().h);
}

void BasicButtonObject::onButtonReleased() {
  // log_(LogLevel::INFO, "State changed from Pressed to Hovered");
  state_manager.stateTransition(ButtonState::Hovered);
  setWidth(state_manager.activeStateObject().renderRect().w);
  setHeight(state_manager.activeStateObject().renderRect().h);
}

void BasicButtonObject::onMouseHovered() {
  // log_(LogLevel::INFO, "State changed from Normal to Hovered");
  state_manager.stateTransition(ButtonState::Hovered);
  setWidth(state_manager.activeStateObject().renderRect().w);
  setHeight(state_manager.activeStateObject().renderRect().h);
}

void BasicButtonObject::onMouseUnhovered() {
  // log_(LogLevel::INFO, "State changed from Hovered to Normal");
  state_manager.stateTransition(ButtonState::Normal);
  setWidth(state_manager.activeStateObject().renderRect().w);
  setHeight(state_manager.activeStateObject().renderRect().h);
}

}  // namespace Truffle

#endif  // Truffle_BUTTON_H
