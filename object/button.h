/**
 * @file      button.h
 * @author    Rei Shimizu (shikugawa) <shikugawa@gmail.com>
 * @brief     Truffle button object
 *
 * @copyright Copyright 2021 Rei Shimizu. All rights reserved.
 */

#ifndef TRUFFLE_BUTTON_H
#define TRUFFLE_BUTTON_H

#include <SDL2/SDL.h>

#include "common/stateful_object_manager.h"
#include "image.h"

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

  StatefulObjectManager<Image, ButtonState> state_manager;

 private:
  bool isMouseHovered(const SDL_Rect& render_rect);
  bool isMouseUnhovered(const SDL_Rect& render_rect);
  bool isPressed(SDL_Event& ev, const SDL_Rect& render_rect);
  bool isReleased(SDL_Event& ev, const SDL_Rect& render_rect);
};

/**
 * 基礎的なボタンの機能を提供するクラス
 */
class Button : public TruffleVisibleObject, public ButtonCallback {
 public:
  /**
   * ボタンのコンストラクタ
   *
   * @param name 名前
   * @param x x座標
   * @param y y座標
   * @param path_normal 初期状態のテクスチャのパス
   * @param path_hovered ホバー状態のテクスチャのパス
   * @param path_pressed 押下時のテクスチャのパス
   */
  Button(std::string name, int x, int y, std::string path_normal,
         std::string path_hovered = "", std::string path_pressed = "");

  // Renderable
  void render() override final;

  // ButtonEventCallback
  virtual void onButtonPressed() override;
  virtual void onButtonReleased() override;
  virtual void onMouseHovered() override;
  virtual void onMouseUnhovered() override;
};

}  // namespace Truffle

#endif  // TRUFFLE_BUTTON_H