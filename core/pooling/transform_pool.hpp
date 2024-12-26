/************************************************************************
 * Copyright (c) 2024 Alvaro Cabrera Barrio
 * This code is licensed under MIT license (see LICENSE.txt for details)
 ************************************************************************/
/**
 * @file transform_pool.hpp
 * @version 1.0
 * @date 29/10/2024
 * @brief Short description
 *
 * Longer description
 */

#pragma once

#include "common/common.hpp"
#include "math/math.hpp"


namespace reveal3d::core::transform {

namespace detail {

struct Transform {
  math::xvec3 position {0.f, 0.f, 0.f};
  math::xvec3 rotation {0.f, 0.f, 0.f};
  math::xvec3 scale {1.f, 1.f, 1.f};
};

} // namespace detail

class Pool {
public:
  using init_info  = detail::Transform;
  using gpu_stored = std::true_type;

  detail::Transform& posRotScale(id_t const id) { return pos_rot_scale_.at(id::index(id)); }

  math::mat4& world(id_t const id) { return world_mat_.at(id::index(id)); }

  math::mat4& invWorld(id_t const id) { return inv_world_.at(id::index(id)); }

protected:
  [[nodiscard]] index_t countData() const { return pos_rot_scale_.size(); }

  void addData(init_info const& init_info) {
    pos_rot_scale_.push_back(init_info);
    auto& [position, rotation, scale] = pos_rot_scale_.at(countData() - 1);
    world_mat_.emplace_back(transpose(affine_transformation(position, scale, rotation)));
    inv_world_.emplace_back(inverse(world_mat_.at(countData() - 1)));
  }

  void removeData(id_t const id) {
    pos_rot_scale_.unordered_remove(id::index(id));
    world_mat_.unordered_remove(id::index(id));
    inv_world_.unordered_remove(id::index(id));
  }

private:
  utils::vector<math::mat4> world_mat_;
  utils::vector<math::mat4> inv_world_;
  utils::vector<detail::Transform> pos_rot_scale_;
};


} // namespace reveal3d::core::transform
