#include <iostream>
#include <vector>
#include <random>
#include <memory>
#include <iostream>
#include <algorithm>
#include <list>

#include "List.h"
#include "Square.h"
#include "Tree.h"
#include "TreeAllocator.h"



int main() {
    std::string command;
    Tree<int, Square<int>*, Allocators::TreeAllocator<Square<int>, 1000>> figures;
    while (std::cin >> command) {
        if (command == "add") {
            int key;
            std::cin >> key;
            if (figures.Find(key) != figures.end()) {
                std::cout << "Element with such key already exists\n";
                continue;
            }
            Square<int>* new_figure = new Square<int>;
            try {
                std::cin >> *new_figure;
                figures.Insert(key, new_figure);
                std::cout << *new_figure << "\n";
            } catch (std::exception& ex) {
                std::cout << ex.what() << "\n";
            }
        } else if (command == "erase") {
            int key;
            std::cin >> key;
            auto it = figures.Find(key);
            if (it != figures.end()) {
                delete (*it).second;
                figures.Erase(figures.Find(key));
            } else {
                std::cout << "No such element in container\n";
            }
        } else if (command == "size") {
            size_t size = 0;
            for (auto i : figures) {
                size++;
            }
            std::cout << size << "\n";
        } else if (command == "count") {
            size_t required_area;
            std::cin >> required_area;
            std::cout << std::count_if(figures.begin(), figures.end(), [&required_area] (auto fig) {
                return fig.second->Area() < required_area;
            });
        } else if (command == "print") {
            std::for_each(figures.begin(), figures.end(), [] (auto pair) {
                std::cout << "(" << pair.first << ", " << *(pair.second) << ") ";
            });
        } else {
            std::cout << "Incorrect command\n";
            std::cin.ignore(32767, '\n');
        }
    }
    for (auto i : figures) {
        delete i.second;
    }
    return 0;
}