#pragma once

#include <memory>
#include <tuple>
#include <iostream>


template <typename U, typename V>
std::ostream& operator << (std::ostream& os, const std::vector<std::pair<U,V>>& v) {
    for (auto i : v) {
        os << "{" << i.first << ", " << i.second << "} ";
    }
    return os;
}

template <typename Key, typename Value>
struct TreeNode;
template <typename Key, typename Value, typename Allocator = std::allocator<TreeNode<Key, Value>>>
struct TreeIterator;
template <typename Key, typename Value, typename Allocator = std::allocator<TreeNode<Key, Value>>>
class Tree;


template <typename Key, typename Value, typename Allocator>
struct TreeIterator {

    template <typename A, typename B, typename C>
    friend class Tree;

    using value_type = std::pair<const Key&, Value&>;
    using reference = std::pair<const Key&, Value&>&;
    using pointer = std::pair<const Key&, Value&>*;
    using difference_type = ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;

    using tree_type = Tree<Key, Value, Allocator>;
    using iterator_type = TreeIterator<Key,Value, Allocator>;
    using node_type = TreeNode<Key, Value>;

    TreeIterator(std::shared_ptr<node_type> element, tree_type* tree)
    : element_(element), tree_(tree) {}

    iterator_type& operator++ () {
        std::shared_ptr<node_type> locked = element_.lock();
        if (locked == nullptr) {
            throw std::logic_error("Dereferencing of deleted iterator");
        }
        if (locked->right != nullptr) {
            std::shared_ptr<node_type> result = locked->right;
            while (result->left != nullptr) {
                result = result->left;
            }
            element_ = result;
        } else {
            while (locked->parent.lock() != nullptr && locked->parent.lock()->right == locked) {
                locked = locked->parent.lock();
            }
            if (locked->parent.lock() == nullptr) {
                throw std::logic_error("Increment of end iterator");
            }
            element_ = locked->parent;
        }
        return *this;
    }

    iterator_type& operator-- () {
        std::shared_ptr<node_type> locked = element_.lock();
        if (locked == nullptr) {
            throw std::logic_error("Dereferencing of deleted iterator");
        }
        if (locked->left != nullptr) {
            std::shared_ptr<node_type> result = locked->left;
            while (result->right != nullptr) {
                result = result->right;
            }
            element_ = result;
        } else {
            while (locked->parent.lock() != nullptr && locked->parent.lock()->left == locked) {
                locked = locked->parent.lock();
            }
            if (locked->parent.lock() == nullptr) {
                throw std::logic_error("Decrement of begin iterator");
            }
            element_ = locked->parent;
        }
        return *this;
    }

    iterator_type operator++ (int) {
        iterator_type copy = *this;
        ++(*this);
        return copy;
    }

    iterator_type operator-- (int) {
        iterator_type copy = *this;
        --(*this);
        return copy;
    }

    std::pair<const Key&, Value&> operator * () {
        std::shared_ptr<node_type> locked = element_.lock();
        if (locked == nullptr) {
            throw std::logic_error("Dereferencing of deleted iterator");
        }
        if (locked->parent.lock() == nullptr) {
            throw std::logic_error("Dereferencing of end iterator");
        }
        return std::pair<const Key&, Value&>(locked->key, locked->value);
    }

    bool operator == (iterator_type other) {
        return element_.lock() != nullptr && other.element_.lock() != nullptr && element_.lock() == other.element_.lock();
    }

    bool operator != (iterator_type other) {
        return !(*this == other);
    }

private:

    tree_type* tree_;
    std::weak_ptr<node_type> element_;
};

template <typename Key, typename Value>
struct TreeNode {
    TreeNode() = default;
    TreeNode(const Key& new_key, const Value& new_value)
    : key(new_key), value(new_value) {}

    Key key;
    Value value;

    std::weak_ptr<TreeNode<Key, Value>> parent;
    std::shared_ptr<TreeNode<Key, Value>> left = nullptr;
    std::shared_ptr<TreeNode<Key, Value>> right = nullptr;
};

template <typename Key, typename Value, typename Allocator>
class Tree {

    using iterator_type = TreeIterator<Key,Value,Allocator>;
    using node_type = TreeNode<Key, Value>;
    using allocator_type = typename Allocator::template rebind<node_type>::other;


    struct deleter {
        deleter(allocator_type* allocator)
        : allocator_(allocator) {}

        void operator() (node_type* ptr) {
            std::allocator_traits<allocator_type>::destroy(*allocator_, ptr);
            allocator_->deallocate(ptr, 1);
        }

    private:
        allocator_type* allocator_;
    };

public:

    Tree() {
        node_type* new_elem_raw = allocator_.allocate(1);
        std::allocator_traits<allocator_type>::construct(allocator_, new_elem_raw);
        terminator_ = std::shared_ptr<node_type>(new_elem_raw, deleter(&allocator_));
    }

    iterator_type Find(const Key& elem) {
        if (Empty()) {
            return end();
        }
        std::shared_ptr<node_type> cur_ptr = terminator_->left;
        while(cur_ptr != nullptr) {
            if (elem == cur_ptr->key) {
                return iterator_type(cur_ptr, this);
            } else if (elem > cur_ptr->key) {
                cur_ptr = cur_ptr->right;
            } else if (elem < cur_ptr->key) {
                cur_ptr = cur_ptr->left;
            }
        }
        return end();
    }

    iterator_type LowerBound(const Key& elem) {
        if (Empty()) {
            return end();
        }
        std::shared_ptr<node_type> bigger = nullptr;
        std::shared_ptr<node_type> cur_ptr = terminator_->left;
        while (cur_ptr != nullptr) {
            if (elem == cur_ptr->key) {
                return iterator_type(cur_ptr, this);
            } else if (elem > cur_ptr->key) {
                cur_ptr = cur_ptr->right;
            } else if (elem < cur_ptr->key) {
                if (bigger == nullptr || cur_ptr->key < bigger->key) {
                    bigger = cur_ptr;
                }
                cur_ptr = cur_ptr->left;
            }
        }
        if (bigger == nullptr) {
            return end();
        }
        return iterator_type(bigger, this);

    }

    iterator_type Insert(const Key& elem_key, const Value& elem_value) {
        if (Empty()) {
            // если пустое заменить корень
            node_type* new_elem_raw = allocator_.allocate(1);
            std::allocator_traits<allocator_type>::construct(allocator_, new_elem_raw, elem_key, elem_value);
            terminator_->left = std::shared_ptr<node_type>(new_elem_raw, deleter(&allocator_));
            terminator_->left->parent = terminator_;
            return iterator_type(terminator_->left, this);
        }
        std::shared_ptr<node_type> cur_ptr = terminator_->left;
        while (true) {
            if (elem_key >= cur_ptr->key && cur_ptr->right != nullptr) {
                cur_ptr = cur_ptr->right;
            } else if (elem_key < cur_ptr->key && cur_ptr->left != nullptr) {
                cur_ptr = cur_ptr->left;
            } else {
                break;
            }
        }
        node_type* new_elem_raw = allocator_.allocate(1);
        std::allocator_traits<allocator_type>::construct(allocator_, new_elem_raw, elem_key, elem_value);
        std::shared_ptr<node_type> new_elem(new_elem_raw, deleter(&allocator_));
        if (elem_key >= cur_ptr->key) {
            cur_ptr->right = new_elem;
        } else {
            cur_ptr->left = new_elem;
        }
        new_elem->parent = cur_ptr;
        return iterator_type(new_elem, this);

    }

   

    void Erase(iterator_type elem) {
        if (elem.tree_ != this) {
            throw std::logic_error("Iterator doesnt belong to this container");
        }
        if (elem == end()) {
            throw std::logic_error("Deletion of end iterator");
        }

        std::shared_ptr<node_type> cur_elem = elem.element_.lock();

        if (cur_elem == nullptr) {
            throw std::logic_error("Use of deleted iterator");
        }
        std::shared_ptr<node_type> parent = cur_elem->parent.lock();
        if (cur_elem->right == nullptr) {
            if (cur_elem == parent->left) {
                parent->left = cur_elem->left;
            } else {
                parent->right = cur_elem->left;
            }
            if (cur_elem->left != nullptr) {
                cur_elem->left->parent = parent;
            }
        } else if (cur_elem->right->left == nullptr) {
            cur_elem->right->left = cur_elem->left;
            if (cur_elem->left != nullptr) {
                cur_elem->left->parent = cur_elem->right;
            }
            cur_elem->right->parent = parent;
            if (cur_elem == parent->left) {
                parent->left = cur_elem->right;
            } else {
                parent->right = cur_elem->right;
            }

        } else {
            std::shared_ptr<node_type> replacer = cur_elem->right;
            while (replacer->left != nullptr) {
                replacer = replacer->left;
            }
            replacer->parent.lock()->left = replacer->right;
            if (replacer->right != nullptr) {
                replacer->right->parent = replacer->parent;
            }
            replacer->left = cur_elem->left;
            if (cur_elem->left != nullptr) {
                cur_elem->left->parent = replacer;
            }
            replacer->right = cur_elem->right;
            cur_elem->right->parent = replacer;
            replacer->parent = parent;
            if (cur_elem == parent->left) {
                parent->left = replacer;
            } else {
                parent->right = replacer;
            }
        }

    }

    bool Empty() const {
        return terminator_->left == nullptr;
    }

    iterator_type begin() {
        std::shared_ptr<node_type> result = terminator_;
        while (result->left != nullptr) {
            result = result->left;
        }
        return iterator_type(result, this);
    }

    iterator_type end() {
        return iterator_type(terminator_, this);
    }


private:
    allocator_type allocator_;
    std::shared_ptr<node_type> terminator_ = nullptr;
};