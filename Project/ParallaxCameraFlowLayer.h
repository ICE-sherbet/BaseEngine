﻿// @ParallaxCameraFlowLayer.h
// @brief
// @author
// @date 2023/01/07
//
// @details

#pragma once
#include <Graphics/Texture.h>

#include "Actor.h"

class ParallaxCameraFlowLayer {
 public:
  using CreateInfo = struct {
    /**
     * \brief 構造名
     */
    std::string_view literal_name;
    /**
     * \brief 0から1のカメラ追従割合(0で完全停止 1でカメラと同じ座標)
     */
    float follow_per_x;
    float follow_per_y;

    int draw_order;
    bool vertical_loop;
    base_engine::Vector2 initial_position;
  };

  static base_engine::Actor* Create(base_engine::Game* game,
                                    std::string_view name,
                                    const std::span<CreateInfo>& literals);
};
