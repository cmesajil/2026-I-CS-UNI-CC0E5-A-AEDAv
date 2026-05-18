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
#include <sstream>
using namespace std;

// Declaraciones adelantadas de los iteradores y el árbol para las relaciones de amistad
template<typename Container, bool Reverse> class InorderIterator;
template<typename Container, bool Reverse> class PreorderIterator;
template<typename Container, bool Reverse> class PostorderIterator;
template<typename Trait> class BinaryTree;

// =============================================================================
// NODO BASE DEL ÁRBOL BINARIO (ENCAPSULADO)
// =============================================================================

template<typename T, typename Derived>
class BinaryTreeNodeBase {
private:
    T m_data;
    Ref m_ref;
    Derived* parent;
    Derived* m_pChild[2];

    // Otorgamos acceso exclusivo a la clase derivada (CRTP)
    friend Derived;

public:
    using value_type = T;

    BinaryTreeNodeBase(T data, Ref ref)
        : m_data(data),
          m_ref(ref),
          parent(nullptr),
          m_pChild{nullptr, nullptr}
    {}

    // Getters y Setters públicos
    Derived* getChild(int index) const { return m_pChild[index]; }
    void setChild(int index, Derived* child) { m_pChild[index] = child; }

    Derived* getParent() const { return parent; }
    void setParent(Derived* p) { parent = p; }

    const T& getData() const { return m_data; }
    T& getDataRef() { return m_data; }
    void setData(const T& data) { m_data = data; }

    Ref getRef() const { return m_ref; }
    void setRef(Ref ref) { m_ref = ref; }
};

// =============================================================================
// NODO DERIVADO DEL ÁRBOL BINARIO
// =============================================================================

template<typename T>
class BinaryTreeNode : public BinaryTreeNodeBase<T, BinaryTreeNode<T>> {
public:
    using Base = BinaryTreeNodeBase<T, BinaryTreeNode<T>>;
    using Base::Base;
    using value_type = T;

    // Relaciones de amistad para que los iteradores y el árbol accedan a la base de forma directa y rápida
    template<typename Container, bool Reverse>
    friend class InorderIterator;

    template<typename Container, bool Reverse>
    friend class PreorderIterator;

    template<typename Container, bool Reverse>
    friend class PostorderIterator;

    template<typename Trait>
    friend class BinaryTree;

    template<typename U>
    friend ostream& operator<<(ostream& os, BinaryTreeNode<U>& node);
};

template <typename T>
ostream& operator<<(ostream& os, BinaryTreeNode<T>& node) {
    // Uso de getters públicos para máxima pulcritud de encapsulamiento
    return os << "(" << node.getData() << ", " << node.getRef() << ")";
}

template <typename T>
struct AscendingBinaryTreeTrait : public BaseTrait<BinaryTreeNode<T>, std::less<T>> {};

template <typename T>
struct DescendingBinaryTreeTrait : public BaseTrait<BinaryTreeNode<T>, std::greater<T>> {};

// =============================================================================
// ITERADORES PARAMETRIZADOS (SOPORTAN PRIVATE VIA GETTERS/SETTERS)
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

        if (current->getChild(RIGHT)) {
            current = current->getChild(RIGHT);
            while (current->getChild(LEFT)) current = current->getChild(LEFT);
        } else {
            Node* parentNode = current->getParent();
            while (parentNode && current == parentNode->getChild(RIGHT)) {
                current = parentNode;
                parentNode = parentNode->getParent();
            }
            current = parentNode;
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

        if (current->getChild(LEFT)) current = current->getChild(LEFT);
        else if (current->getChild(RIGHT)) current = current->getChild(RIGHT);
        else {
            Node* parentNode = current->getParent();
            while (parentNode) {
                if (current == parentNode->getChild(LEFT) && parentNode->getChild(RIGHT)) {
                    this->m_pNode = parentNode->getChild(RIGHT);
                    return *this;
                }
                current = parentNode;
                parentNode = parentNode->getParent();
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
            if (node->getChild(LEFT)) node = node->getChild(LEFT);
            else if (node->getChild(RIGHT)) node = node->getChild(RIGHT);
            else break;
        }
        return node;
    }

    MySelf& operator++() {
        if (!this->m_pNode) return *this;
        Node* current = this->m_pNode;
        Node* parentNode = current->getParent();

        if (!parentNode) { this->m_pNode = nullptr; return *this; }

        if (current == parentNode->getChild(LEFT) && parentNode->getChild(RIGHT)) current = deepest(parentNode->getChild(RIGHT));
        else current = parentNode;

        this->m_pNode = current;
        return *this;
    }
};

// =============================================================================
// CLASE BINARY TREE (MODIFICADA CON LOCK SEGURO Y SETTERS)
// =============================================================================

template<typename Trait>
class BinaryTree {
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;
    using Myself     = BinaryTree<Trait>;
protected:
    Node *m_pRoot;
    Comp m_comp;

protected:
    mutable shared_mutex m_mtx;
    void internal_insert(Node*& pNode, Node* parent, value_type data, Ref ref);

    void internal_clear(Node* node) {
        if(!node) return;
        internal_clear(node->getChild(0));
        internal_clear(node->getChild(1));
        delete node;
    }

    Node* internal_clone(Node* node, Node* parent = nullptr) {
        if(!node) return nullptr;
        Node* newNode = new Node(node->getData(), node->getRef());
        newNode->setParent(parent);
        newNode->setChild(0, internal_clone(node->getChild(0), newNode));
        newNode->setChild(1, internal_clone(node->getChild(1), newNode));
        return newNode;
    }

public:
    BinaryTree() : m_pRoot(nullptr) {}
    ~BinaryTree() { clear(); }

    void clear() {
        unique_lock<shared_mutex> lock(m_mtx);
        internal_clear(m_pRoot);
        m_pRoot = nullptr;
    }

    BinaryTree(const BinaryTree& other) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = internal_clone(other.m_pRoot);
    }

    BinaryTree(BinaryTree&& other) noexcept {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = std::exchange(other.m_pRoot, nullptr);
    }

    using InorderForwardIterator   = InorderIterator<Myself, false>;
    using InorderBackwardIterator  = InorderIterator<Myself, true>;
    using const_InorderForwardIterator  = InorderIterator<const Myself, false>;
    friend const_InorderForwardIterator;
    using PreorderForwardIterator  = PreorderIterator<Myself, false>;
    using PreorderBackwardIterator = PreorderIterator<Myself, true>;
    using PostorderForwardIterator = PostorderIterator<Myself, false>;
    using PostorderBackwardIterator = PostorderIterator<Myself, true>;

    friend InorderForwardIterator;   friend InorderBackwardIterator;
    friend PreorderForwardIterator;  friend PreorderBackwardIterator;
    friend PostorderForwardIterator; friend PostorderBackwardIterator;

    InorderForwardIterator begin() {
        Node* c = m_pRoot;
        while(c && c->getChild(0)) c = c->getChild(0);
        return InorderForwardIterator(this, c); }
    InorderForwardIterator end()   { return InorderForwardIterator(this, nullptr); }
    const_InorderForwardIterator begin() const {
        Node* c = m_pRoot;
        while(c && c->getChild(0)) c = c->getChild(0);
        return const_InorderForwardIterator(this, c); }
    const_InorderForwardIterator end() const   { return const_InorderForwardIterator(this, nullptr); }

    InorderBackwardIterator rbegin() {
        Node* c = m_pRoot;
        while(c && c->getChild(1)) c = c->getChild(1);
        return InorderBackwardIterator(this, c); }
    InorderBackwardIterator rend()   { return InorderBackwardIterator(this, nullptr); }

    PreorderForwardIterator begin_preorder() { return PreorderForwardIterator(this, m_pRoot); }
    PreorderForwardIterator end_preorder()   { return PreorderForwardIterator(this, nullptr); }

    PreorderBackwardIterator rbegin_preorder() { return PreorderBackwardIterator(this, m_pRoot); }
    PreorderBackwardIterator rend_preorder()   { return PreorderBackwardIterator(this, nullptr); }

    PostorderForwardIterator begin_postorder() { return PostorderForwardIterator(this, PostorderForwardIterator::deepest(m_pRoot)); }
    PostorderForwardIterator end_postorder()   { return PostorderForwardIterator(this, nullptr); }

    PostorderBackwardIterator rbegin_postorder() { return PostorderBackwardIterator(this, PostorderBackwardIterator::deepest(m_pRoot)); }
    PostorderBackwardIterator rend_postorder()   { return PostorderBackwardIterator(this, nullptr); }

    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&... args) {
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_pRoot == nullptr) return;
        for(auto& item : *this) func(item, std::forward<Args>(args)...);
    }

    std::string toString() const {
        std::ostringstream oss;
        oss << *this;
        return oss.str();
    }

    template<typename IteratorBegin, typename IteratorEnd>
    void print_range(IteratorBegin beginIt, IteratorEnd endIt)
    {
        for(auto it = beginIt; it != endIt; ++it)
            std::cout << *(it.getNode()) << " ";

        std::cout << std::endl;
    }

    void print_inorder() {print_range(begin(), end()); }
    void print_preorder() { print_range(begin_preorder(), end_preorder()); }
    void print_postorder() { print_range(begin_postorder(), end_postorder()); }

    void print_reverse_inorder() { print_range(rbegin(), rend()); }
    void print_reverse_preorder() { print_range(rbegin_preorder(), rend_preorder()); }
    void print_reverse_postorder() { print_range(rbegin_postorder(), rend_postorder()); }

    friend std::ostream& operator<<(std::ostream& os, const BinaryTree& Btn) {
        std::shared_lock<std::shared_mutex> lock(Btn.m_mtx);
        os << "[";
        for(auto it = Btn.begin(); it != Btn.end(); ++it){
            os << *(it.getNode());
            auto nextIt = it;
            ++nextIt;
            if(nextIt != Btn.end()) os << ",";
        }
        return os << "]";
    }

    friend std::istream& operator>>(std::istream& is, BinaryTree& Btn) {
        return read_list(is, Btn);
    }

    void insert(value_type data, Ref ref) {
        unique_lock<shared_mutex> lock(m_mtx);
        internal_insert(m_pRoot, nullptr, data, ref);
    }
};

template<typename Trait>
void BinaryTree<Trait>::internal_insert(Node*& pNode, Node* parent, value_type data, Ref ref) {
    if (pNode == nullptr) {
        pNode = new Node(data, ref);
        pNode->setParent(parent);
        return;
    }

    auto branch = m_comp(pNode->getData(), data);

    // Pasamos la referencia al puntero del hijo usando la función pública getChild
    Node* child = pNode->getChild(branch);
    internal_insert(child, pNode, data, ref);
    pNode->setChild(branch, child); // Aseguramos que el puntero se actualice en el nodo
}

#endif // __BINARYTREE_H__
