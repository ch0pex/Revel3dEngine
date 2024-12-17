/************************************************************************
 * Copyright (c) 2024 Alvaro Cabrera Barrio
 * This code is licensed under MIT license (see LICENSE.txt for details)
 ************************************************************************/
/**
 * @file component.hpp
 * @version 1.0
 * @date 15/12/2024
 * @brief Short description
 *
 * Longer description
 */

#pragma once


#include "core/scene.hpp"

namespace reveal3d::core {

template<typename T>
class Component {
public:
  constexpr Component() : id_(id::invalid) { }

  constexpr Component(id_t const id) : id_(id) { }

  [[nodiscard]] constexpr bool isAlive() const { return id_ != id::invalid; }

  [[nodiscard]] constexpr id_t id() const { return id_; }

  [[nodiscard]] constexpr index_t entityIdx() const { return pool().getMappedId(id_); }

  static constexpr auto pool() -> decltype(auto) { return scene.pool<T>(); };

protected:
  id_t id_;
};


} // namespace reveal3d::core
