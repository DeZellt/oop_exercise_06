#pragma once

#include <iostream>
#include "Tree.h"

namespace Allocators {

template <typename T, size_t MEM_SIZE>
class TreeAllocator {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using is_always_equal = std::false_type;

    template<class V>
    struct rebind {
        using other = TreeAllocator<V, MEM_SIZE>;
    };

    TreeAllocator() {
        pool_ = (char*) malloc(MEM_SIZE);
        free_blocks_.Insert(pool_, MEM_SIZE);
    }

    ~TreeAllocator() {
        free(pool_);
    }

    TreeAllocator(const TreeAllocator &) = delete;
    TreeAllocator(TreeAllocator &&) = delete;

    T* allocate(size_t alloc_size) {
        if (alloc_size == 0) {
            throw std::logic_error("Allocation of 0 bytes");
        }
        alloc_size *= sizeof(T);
        auto iter = free_blocks_.begin();
        while (iter != free_blocks_.end() && (*iter).second < alloc_size) {
            ++iter;
        }
        if (iter == free_blocks_.end()) {
            throw std::bad_alloc();
        }
        char* block_ptr = (*iter).first;
        size_t block_size = (*iter).second;
        if (alloc_size == block_size) {
            free_blocks_.Erase(iter);
            busy_blocks_.Insert(block_ptr, block_size);
        } else {
            free_blocks_.Erase(iter);
            free_blocks_.Insert(block_ptr + alloc_size, block_size - alloc_size);
            busy_blocks_.Insert(block_ptr, alloc_size);
        }
        Print();
        return (T*)block_ptr;
    }

    void deallocate(T* ptr, size_t size) {
        size *= sizeof(T);
        auto iter = busy_blocks_.Find((char*)ptr);
        if (iter == busy_blocks_.end() || size != (*iter).second) {
            throw std::logic_error("Wrong ptr to deallocate");
        }
        busy_blocks_.Erase(iter);

        auto deallocated_iter = free_blocks_.Insert((char*) ptr, size);
        if (deallocated_iter != free_blocks_.begin()) {
            auto prev_iter = std::prev(deallocated_iter);
            char* prev_ptr = (*prev_iter).first;
            size_t prev_size = (*prev_iter).second;
            if (prev_ptr + prev_size == (char*)ptr) {
                (*prev_iter).second += size;
                free_blocks_.Erase(deallocated_iter);
                deallocated_iter = prev_iter;
            }
        }
        if (deallocated_iter != free_blocks_.end()) {
            auto next_iter = std::next(deallocated_iter);
            char* next_ptr = (*next_iter).first;
            size_t next_size = (*next_iter).second;
            if ((char*)ptr + size == next_ptr) {
                (*deallocated_iter).second += next_size;
                free_blocks_.Erase(next_iter);
            }
        }

        Print();
    }

    void Print() {
        std::cout << "free\n";
        for (auto pair : free_blocks_) {
            std::cout << "(" << pair.second << ") ";
        }
        std::cout << "\n";
        std::cout << "busy\n";
        for (auto pair : busy_blocks_) {
            std::cout << "(" << pair.second << ") ";
        }
        std::cout << "\n";
    }

private:
    Tree<char*, size_t> free_blocks_;
    Tree<char*, size_t> busy_blocks_;
    char* pool_;
};
}