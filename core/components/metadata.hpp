/************************************************************************
 * Copyright (c) 2024 Alvaro Cabrera Barrio
 * This code is licensed under MIT license (see LICENSE.txt for details)
 ************************************************************************/
/**
 * @file geometry.hpp
 * @version 1.0
 * @date 11/03/2024
 * @brief Short description
 *
 * Geometry entity component
 */

#pragma once

#include "pool.hpp"

#include <string>

namespace reveal3d::core {

class Metadata {
public:
    using InitInfo = std::string;

    struct Pool {
        u32 count() { return names.size(); }
        utl::vector<std::string> names;
        utl::vector<std::string> comments;
        utl::vector<std::string> dates;
    };

    struct Data {
        std::string_view name;
        std::string_view comment;
        std::string_view date;
    };

    Metadata() : id_(id::invalid) {}
    explicit Metadata(id_t id);

    std::string& name();
    std::string& comment();
    std::string& date();

    [[nodiscard]] inline bool isAlive() const { return id_ != id::invalid; }
    [[nodiscard]] inline id_t id() const { return id_; }
    void update();
    Data data();
private:
    id_t id_;
};

template<>
inline Metadata Pool<Metadata>::addComponent(id_t id) {
    const id_t metadata_id { id_factory_.New(id::index(id)) };

    data_.names.emplace_back("Entity_" + std::to_string(id));
    data_.comments.emplace_back();
    data_.dates.emplace_back("10/12/2024");  //TODO
    data_.comments.at(data_.comments.size() - 1U).reserve(1024);
    add(id::index(id), metadata_id);

    return Metadata(metadata_id);
}

template<>
inline Metadata Pool<Metadata>::addComponent(id_t id, Metadata::InitInfo &&init_info) {
    const id_t metadata_id { id_factory_.New(id::index(id)) };

    data_.names.emplace_back(std::move(init_info));
    data_.comments.emplace_back();
    data_.dates.emplace_back("10/12/2024");  //TODO
    add(id::index(id), metadata_id);

    assert(id::index(metadata_id) < data_.count());

    return Metadata(metadata_id);
}

template<>
inline void Pool<Metadata>::removeComponent(id_t id) {
    const id_t metadata_id {components_.at(id::index(id)).id() };
    const u32 idx { id::index(metadata_id) };
    data_.names.unordered_remove(idx);
    data_.comments.unordered_remove(idx);
    data_.dates.unordered_remove(idx);
    remove(metadata_id);
}

}
