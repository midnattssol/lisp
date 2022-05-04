#include <cstddef>
#include <vector>

/* A simple tree template. */
template <class T>
class Tree {
   public:
    std::vector<T> nodes;
    std::vector<unsigned int> depths;

    size_t size() { return this->nodes.size(); }

    /* Returns a new subtree from a node and its children. */
    Tree<T> subtree(unsigned int index) {
        Tree<T> result;
        auto base_depth = this->depths[index];
        auto size = this->size();

        result.nodes.push_back(nodes[index]);
        result.depths.push_back(0);

        for (size_t i = index + 1; (i < size) && (depths[i] > base_depth);
             i++) {
            result.nodes.push_back(nodes[i]);
            result.depths.push_back(depths[i] - base_depth);
        }
        return result;
    }

    bool operator==(Tree<T> *tree) {
        unsigned int size = this->size();
        for (size_t i = 0; i < size; i++) {
            if (this->depths[i] != tree->depths[i]) { return false; }
            if (this->nodes[i] != tree->nodes[i]) { return false; }
        }
        return true;
    }

    bool operator!=(Tree<T> *tree) { return !(*this == tree); }
};
