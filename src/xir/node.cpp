#include <luisa/core/logging.h>
#include <luisa/xir/node.h>

namespace luisa::compute::xir {

void Node::insert_before_self(Node *node) noexcept {
    LUISA_ASSERT(!is_head_sentinel(), "Cannot insert before head sentinel.");
    LUISA_ASSERT(!node->is_linked(), "Node to insert is already linked.");
    node->prev = prev;
    node->next = this;
    prev->next = node;
    prev = node;
}

void Node::insert_after_self(Node *node) noexcept {
    LUISA_ASSERT(!is_tail_sentinel(), "Cannot insert after tail sentinel.");
    LUISA_ASSERT(!node->is_linked(), "Node to insert is already linked.");
    node->prev = this;
    node->next = next;
    next->prev = node;
    next = node;
}

void Node::remove_self() noexcept {
    LUISA_ASSERT(!is_sentinel(), "Cannot remove sentinel.");
    LUISA_ASSERT(is_linked(), "Node to remove is not linked.");
    prev->next = next;
    next->prev = prev;
    prev = nullptr;
    next = nullptr;
}

void Node::replace_self_with(Node *node) noexcept {
    LUISA_ASSERT(!is_sentinel(), "Cannot replace sentinel.");
    LUISA_ASSERT(!node->is_linked(), "Node to insert is already linked.");
    node->prev = prev;
    node->next = next;
    prev->next = node;
    next->prev = node;
    prev = nullptr;
    next = nullptr;
}

}// namespace luisa::compute::xir
