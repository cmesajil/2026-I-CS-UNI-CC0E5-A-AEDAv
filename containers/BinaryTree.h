#ifndef __BINARYTREE_H__
#define __BINARYTREE_H__

#include <iostream>
#include <cstddef>
#include <string>
#include <fstream>
#include <thread>
#include "../types.h"
#include <shared_mutex>
#include "traits.h"
#include <mutex>
#include "general_iterator.h"

using namespace std;

// =============================================================================
// ITERADORES PARAMETRIZADOS (FORWARD / BACKWARD VIA TEMPLATE)
// =============================================================================

template<typename Container, bool Reverse = false>
class InorderIterator : public general_iterator<Container, InorderIterator<Container, Reverse>> {
    static constexpr int LEFT = Reverse ? 1 : 0;
    static constexpr int RIGHT = Reverse ? 0 : 1;
public:
    using MySelf = InorderIterator<Container, Reverse>;
    using Parent = general_iterator<Container, MySelf>;
    using Node = typename Container::Node;
    using Parent::Parent;

    MySelf& operator++() {
        if (!this->m_pNode) return *this;
        Node* current = this->m_pNode;

        if (current->m_pChild[RIGHT]) {
            current = current->m_pChild[RIGHT];
            while (current->m_pChild[LEFT]) current = current->m_pChild[LEFT];
        } else {
            Node* parent = current->parent;
            while (parent && current == parent->m_pChild[RIGHT]) {
                current = parent;
                parent = parent->parent;
            }
            current = parent;
        }
        this->m_pNode = current;
        return *this;
    }
};

template<typename Container, bool Reverse = false>
class PreorderIterator : public general_iterator<Container, PreorderIterator<Container, Reverse>> {
    static constexpr int LEFT = Reverse ? 1 : 0;
    static constexpr int RIGHT = Reverse ? 0 : 1;
public:
    using MySelf = PreorderIterator<Container, Reverse>;
    using Parent = general_iterator<Container, MySelf>;
    using Node = typename Container::Node;
    using Parent::Parent;

    MySelf& operator++() {
        if (!this->m_pNode) return *this;
        Node* current = this->m_pNode;

        if (current->m_pChild[LEFT]) current = current->m_pChild[LEFT];
        else if (current->m_pChild[RIGHT]) current = current->m_pChild[RIGHT];
        else {
            Node* parent = current->parent;
            while (parent) {
                if (current == parent->m_pChild[LEFT] && parent->m_pChild[RIGHT]) {
                    this->m_pNode = parent->m_pChild[RIGHT];
                    return *this;
                }
                current = parent;
                parent = parent->parent;
            }
            current = nullptr;
        }
        this->m_pNode = current;
        return *this;
    }
};

template<typename Container, bool Reverse = false>
class PostorderIterator : public general_iterator<Container, PostorderIterator<Container, Reverse>> {
    static constexpr int LEFT = Reverse ? 1 : 0;
    static constexpr int RIGHT = Reverse ? 0 : 1;
public:
    using MySelf = PostorderIterator<Container, Reverse>;
    using Parent = general_iterator<Container, MySelf>;
    using Node = typename Container::Node;
    using Parent::Parent;

    static Node* deepest(Node* node) {
        while (node) {
            if (node->m_pChild[LEFT]) node = node->m_pChild[LEFT];
            else if (node->m_pChild[RIGHT]) node = node->m_pChild[RIGHT];
            else break;
        }
        return node;
    }

    MySelf& operator++() {
        if (!this->m_pNode) return *this;
        Node* current = this->m_pNode;
        Node* parent = current->parent;

        if (!parent) { this->m_pNode = nullptr; return *this; }

        if (current == parent->m_pChild[LEFT] && parent->m_pChild[RIGHT]) current = deepest(parent->m_pChild[RIGHT]);
        else current = parent;

        this->m_pNode = current;
        return *this;
    }
};

// =============================================================================
// NODO DEL ÁRBOL BINARIO
// =============================================================================

template<typename T>
struct BinaryTreeNode {
    T m_data;
    Ref m_ref;
    BinaryTreeNode *parent;
    BinaryTreeNode *m_pChild[2];

    using value_type = T;
    BinaryTreeNode(T data, Ref ref) : m_data(data), m_ref(ref), parent(nullptr), m_pChild{nullptr, nullptr} {}

    BinaryTreeNode* getChild(int index) const { return m_pChild[index]; }
    BinaryTreeNode* getParent() const { return parent; }
    T getData() const { return m_data; }
    T& getDataRef() { return m_data; }
    void setData(T data) { m_data = data; }
    Ref getRef() const { return m_ref; }
    void setRef(Ref ref) { m_ref = ref; }
};

template <typename T>
ostream& operator<<(ostream& os, BinaryTreeNode<T>& node) {
    return os << "(" << node.m_data << ", " << node.getRef() << ")";
}

template <typename T>
struct AscendingBinaryTreeTrait : public BaseTrait<BinaryTreeNode<T>, std::less<T>> {};

template <typename T>
struct DescendingBinaryTreeTrait : public BaseTrait<BinaryTreeNode<T>, std::greater<T>> {};

// =============================================================================
// CLASE BINARY TREE
// =============================================================================

template<typename Trait>
class BinaryTree {
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;
    using Myself     = BinaryTree<Trait>;
private:
    Node *m_pRoot;
    Comp m_comp;

protected:
    mutable shared_mutex m_mtx;
    void internal_insert(Node*& pNode, Node* parent, value_type data, Ref ref);
    void internal_clear(Node* node) {
        if(!node) return;

        internal_clear(node->m_pChild[0]);
        internal_clear(node->m_pChild[1]);

        delete node;
    }
    Node* internal_clone(Node* node, Node* parent = nullptr) {
        if(!node) return nullptr;

        Node* newNode = new Node(node->m_data, node->m_ref);

        newNode->parent = parent;

        newNode->m_pChild[0] = internal_clone(node->m_pChild[0], newNode);
        newNode->m_pChild[1] = internal_clone(node->m_pChild[1], newNode);

        return newNode;
    }

public:
    BinaryTree() : m_pRoot(nullptr) {}
    ~BinaryTree() {
        clear();
    }
    void clear() {
        unique_lock<shared_mutex> lock(m_mtx);

        internal_clear(m_pRoot);
        m_pRoot = nullptr;
    }

    //copy constructor
    BinaryTree(const BinaryTree& other) {
        shared_lock<shared_mutex> lock(other.m_mtx);

        m_pRoot = internal_clone(other.m_pRoot);
    }

    //move constructor
    BinaryTree(BinaryTree&& other) noexcept {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = std::exchange(other.m_pRoot, nullptr);
    }


    // Alias de todos los iteradores (Forward y Backward)
    using InorderForwardIterator   = InorderIterator<Myself, false>;
    using InorderBackwardIterator  = InorderIterator<Myself, true>;
    using PreorderForwardIterator  = PreorderIterator<Myself, false>;
    using PreorderBackwardIterator = PreorderIterator<Myself, true>;
    using PostorderForwardIterator = PostorderIterator<Myself, false>;
    using PostorderBackwardIterator = PostorderIterator<Myself, true>;

    friend InorderForwardIterator;   friend InorderBackwardIterator;
    friend PreorderForwardIterator;  friend PreorderBackwardIterator;
    friend PostorderForwardIterator; friend PostorderBackwardIterator;

    // Métodos Inorder Forward
    InorderForwardIterator begin() { Node* c = m_pRoot; while(c && c->m_pChild[0]) c = c->m_pChild[0]; return InorderForwardIterator(this, c); }
    InorderForwardIterator end()   { return InorderForwardIterator(this, nullptr); }
    InorderForwardIterator begin() const { Node* c = m_pRoot; while(c && c->m_pChild[0]) c = c->m_pChild[0]; return InorderForwardIterator(const_cast<Myself*>(this), c); }
    InorderForwardIterator end() const   { return InorderForwardIterator(const_cast<Myself*>(this), nullptr); }

    // Métodos Inorder Backward
    InorderBackwardIterator rbegin() { Node* c = m_pRoot; while(c && c->m_pChild[1]) c = c->m_pChild[1]; return InorderBackwardIterator(this, c); }
    InorderBackwardIterator rend()   { return InorderBackwardIterator(this, nullptr); }

    // Métodos Preorder Forward
    PreorderForwardIterator begin_preorder() { return PreorderForwardIterator(this, m_pRoot); }
    PreorderForwardIterator end_preorder()   { return PreorderForwardIterator(this, nullptr); }

    // Métodos Preorder Backward
    PreorderBackwardIterator rbegin_preorder() { return PreorderBackwardIterator(this, m_pRoot); }
    PreorderBackwardIterator rend_preorder()   { return PreorderBackwardIterator(this, nullptr); }

    // Métodos Postorder Forward
    PostorderForwardIterator begin_postorder() { return PostorderForwardIterator(this, PostorderForwardIterator::deepest(m_pRoot)); }
    PostorderForwardIterator end_postorder()   { return PostorderForwardIterator(this, nullptr); }

    // Métodos Postorder Backward
    PostorderBackwardIterator rbegin_postorder() { return PostorderBackwardIterator(this, PostorderBackwardIterator::deepest(m_pRoot)); }
    PostorderBackwardIterator rend_postorder()   { return PostorderBackwardIterator(this, nullptr); }

    // Algoritmos comunes
    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&... args) {
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_pRoot == nullptr) return;
        for(auto& item : *this) func(item, std::forward<Args>(args)...);
    }

    void insert(value_type data, Ref ref) { internal_insert(m_pRoot, nullptr, data, ref); }

    // Métodos de impresión
    void print_inorder() { for(auto it = begin(); it != end(); ++it) std::cout << *(it.getNode()) << " "; std::cout << std::endl; }
    void print_preorder() { for(auto it = begin_preorder(); it != end_preorder(); ++it) std::cout << *(it.getNode()) << " "; std::cout << std::endl; }
    void print_postorder() { for(auto it = begin_postorder(); it != end_postorder(); ++it) std::cout << *(it.getNode()) << " "; std::cout << std::endl; }

    // Métodos de impresión Inversa (Backwards)
    void print_reverse_inorder() { for(auto it = rbegin(); it != rend(); ++it) std::cout << *(it.getNode()) << " "; std::cout << std::endl; }
    void print_reverse_preorder() { for(auto it = rbegin_preorder(); it != rend_preorder(); ++it) std::cout << *(it.getNode()) << " "; std::cout << std::endl; }
    void print_reverse_postorder() { for(auto it = rbegin_postorder(); it != rend_postorder(); ++it) std::cout << *(it.getNode()) << " "; std::cout << std::endl; }

    friend std::ostream& operator<<(std::ostream& os, const BinaryTree& Btn) {
        std::shared_lock<std::shared_mutex> lock(Btn.m_mtx);
        os << "[";
        for(auto it = Btn.begin(); it != Btn.end(); ++it){
            os << *(it.getNode());
            auto nextIt = it;
            ++nextIt;
            if(nextIt != Btn.end())
                os << ",";
        }
        return os << "]";
    }

    friend std::istream& operator>>(std::istream& is, BinaryTree& Btn) {
        std::unique_lock<std::shared_mutex> lock(Btn.m_mtx);
        return read_list(is, Btn);
    }
};

template<typename Trait>
void BinaryTree<Trait>::internal_insert(Node*& pNode, Node* parent, value_type data, Ref ref) {
    if (pNode == nullptr) {
        pNode = new Node(data, ref);
        pNode->parent = parent;
        return;
    }
    auto branch = m_comp(pNode->m_data, data);
    internal_insert(pNode->m_pChild[branch], pNode, data, ref);
}

#endif // __BINARYTREE_H__
