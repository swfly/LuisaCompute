#pragma once

namespace luisa::compute::xir {

// intrusive doubly linked list node, base class for all XIR instructions
struct Node {

    Node *prev = nullptr;
    Node *next = nullptr;

    // ctor & dtor
    Node() noexcept = default;
    virtual ~Node() noexcept = default;

    // query
    [[nodiscard]] bool is_head_sentinel() const noexcept { return prev == nullptr && next != nullptr; }
    [[nodiscard]] bool is_tail_sentinel() const noexcept { return next == nullptr && prev != nullptr; }
    [[nodiscard]] bool is_sentinel() const noexcept { return is_head_sentinel() || is_tail_sentinel(); }
    [[nodiscard]] bool is_linked() const noexcept { return prev != nullptr && next != nullptr; }

    // operations
    void insert_before_self(Node *node) noexcept;
    void insert_after_self(Node *node) noexcept;
    void remove_self() noexcept;
    void replace_self_with(Node *node) noexcept;
};

}// namespace luisa::compute::xir
