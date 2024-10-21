/************************************************************************
 * Copyright (c) 2024 Alvaro Cabrera Barrio
 * This code is licensed under MIT license (see LICENSE.txt for details)
 ************************************************************************/
/**
 * @file transform.cpp
 * @version 1.0
 * @date 11/03/2024
 * @brief Short description
 *
 * Longer description
 */

#include "transform.hpp"
#include "core/scene.hpp"

namespace reveal3d::core {

Pool<Transform> &pool() {
    return core::scene.componentPool<Transform>();
}

Transform::Transform(id_t id) : id_(id) { }

math::mat4& Transform::world() const {
    return pool().data().World(id_);
}

math::mat4& Transform::invWorld() const {
    return pool().data().invWorld(id_);
}

math::xvec3 Transform::position() const {
    return pool().data().pos_rot_scale.at(id::index(id_)).position;
}

math::xvec3 Transform::scale() const {
    return pool().data().posRotScale(id_).scale;
}

math::xvec3 Transform::rotation() const {
    return math::vec_to_degrees(pool().data().posRotScale(id_).rotation);
}

math::xvec3 Transform::worldPosition() const {
    math::mat4 worldMat = pool().data().World(id_);
    return worldMat.translation();
}

math::xvec3 Transform::worldScale() const {
    return pool().data().World(id_).scale();
}

math::xvec3 Transform::worldRotation() const {
    return pool().data().World(id_).rotation();
}

void Transform::position(math::xvec3 pos) const {
    pool().data().posRotScale(id_).position = pos;
    setDirty();
}

void Transform::scale(math::xvec3 size) const {
    pool().data().posRotScale(id_).scale = size;
    setDirty();
}

void Transform::rotation(math::xvec3 rot) const {
    pool().data().posRotScale(id_).rotation = math::vec_to_radians(rot);
    setDirty();
}

void Transform::worldPosition(math::xvec3 pos) {
    Transform::Info& trans = pool().data().posRotScale(id_);
    pool().data().World(id_) = math::transpose(math::affine_transformation(pos, trans.scale, trans.rotation));
    core::Entity parent = scene.getNode(id_).parent;
    if (parent.isAlive()) {
        trans.position = math::transpose(parent.component<Transform>().invWorld()) * pos;
    } else {
        trans.position = pos;
    }

    pool().data().invWorld(id_) = math::inverse(pool().data().World(id_));
    pool().dirties().at(id_) = 3;
    pool().dirtyIds().insert(id_);
    updateChilds();
}

void Transform::worldScale(math::xvec3 scale) {
    Transform::Info& trans = pool().data().posRotScale(id_);
    pool().data().World(id_) = math::transpose(math::affine_transformation(trans.position, scale, trans.rotation));
    core::Entity parent = scene.getNode(id_).parent;
    if (parent.isAlive()) {
        trans.scale = parent.component<Transform>().invWorld() * scale;
    } else {
        trans.scale = scale;
    }
    pool().data().invWorld(id_) = math::inverse(pool().data().World(id_));
    pool().dirties().at(id::index(id_)) = 3;
    pool().dirtyIds().insert(id_);
    updateChilds();
}

void Transform::worldRotation(math::xvec3 rot) {
    Transform::Info& trans = pool().data().posRotScale(id_);
    world() = math::transpose(math::affine_transformation(trans.position, trans.scale, rot));
    core::Entity parent = scene.getNode(id_).parent;
    if (parent.isAlive()) {
        trans.rotation = parent.component<Transform>().invWorld() * rot;
    } else {
       trans.rotation = rot;
    }
    pool().data().invWorld(id_) = math::inverse(pool().data().World(id_));
    pool().dirties().at(id::index(id_)) = 3;
    pool().dirtyIds().insert(id_);
    updateChilds();
}

math::mat4 Transform::calcWorld(id_t id){
    Transform::Info& trans = core::scene.componentPool<Transform>().data().posRotScale(id);
    return math::transpose(math::affine_transformation(trans.position, trans.scale, trans.rotation));
}

void Transform::updateChilds() const {
    core::Scene::Node &node = scene.getNode(id_);
    auto children = node.getChildren();
    for (auto child_id : children) {
        const Entity child = Entity(child_id);
        child.component<Transform>().setDirty();
        child.component<Transform>().updateChilds();
    }
}

void Transform::update() const {
    if (pool().dirties().at(id::index(id_)) != 4) return;

    Transform::Info& transform = pool().data().posRotScale(id_);
    core::Scene::Node &currNode = scene.getNode(id::index(id_));

    if (currNode.parent.isAlive()) {
        const id_t parent_id = currNode.parent.id();
        if (pool().dirties().at(id::index(id_)) < 4) {
            math::mat4 parentWorld = pool().data().World(parent_id);
            world() = parentWorld * calcWorld(parent_id);
        } else {
            currNode.parent.component<Transform>().update();
            math::mat4 parentWorld = pool().data().World(parent_id);
            world() = parentWorld * calcWorld(id_);
        }
    } else {
        world() = calcWorld(id_);
    }
    invWorld() = math::transpose(math::inverse(world()));
    --pool().dirties().at(id::index(id_));
}

void Transform::unDirty() const {
    const id_t idx = id::index(id_);
    if (pool().dirties().at(idx) != 0) {
        --pool().dirties().at(idx);
    } else {
        pool().dirties().at(idx) = 0;
    }
}

void Transform::setDirty() const {
    if (dirty() == 4) {
        return;
    }
    if (dirty() == 0) {
        pool().dirtyIds().insert(id_);
    }
    updateChilds();
    pool().dirties().at(id::index(id_)) = 4;
}

u8 Transform::dirty() const {
    return pool().dirties().at(id::index(id_));
}


} // reveal3d::core namespace