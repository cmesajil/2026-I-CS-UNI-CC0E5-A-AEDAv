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
// NODO DEL ÁRBOL BINARIO
// =============================================================================

template<typename T, typename Derived>
struct BinaryTreeNodeBase {

    using value_type = T;

    T m_data;
    Ref m_ref;

    Derived* parent;
    Derived* m_pChild[2];

    BinaryTreeNodeBase(T data, Ref ref)
        : m_data(data),
          m_ref(ref),
          parent(nullptr),
          m_pChild{nullptr, nullptr}
    {}

    Derived* getChild(int index) const {  return m_pChild[index];}
    Derived* getParent()         const {  return parent;}
    T getData()                  const { return m_data;}
    T& getDataRef()             {return m_data;}
    void setData(T data)        {m_data = data;}
    Ref getRef()                const  {return m_ref;}
    void setRef(Ref ref)        {m_ref = ref;}
};

template<typename T>
class BinaryTreeNode :
    public BinaryTreeNodeBase<T, BinaryTreeNode<T>>
{
public:
    using Base =
        BinaryTreeNodeBase<T, BinaryTreeNode<T>>;

    using Base::Base;

    using value_type = T;

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
    // Acceso seguro por encapsulamiento a los métodos públicos o atributos (por ser friend)
    return os << "(" << node.m_data << ", " << node.m_ref << ")";
}

template <typename T>
struct AscendingBinaryTreeTrait : public BaseTrait<BinaryTreeNode<T>, std::less<T>> {};

template <typename T>
struct DescendingBinaryTreeTrait : public BaseTrait<BinaryTreeNode<T>, std::greater<T>> {};

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
            Node* parentNode = current->parent;
            while (parentNode && current == parentNode->m_pChild[RIGHT]) {
                current = parentNode;
                parentNode = parentNode->parent;
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

        if (current->m_pChild[LEFT]) current = current->m_pChild[LEFT];
        else if (current->m_pChild[RIGHT]) current = current->m_pChild[RIGHT];
        else {
            Node* parentNode = current->parent;
            while (parentNode) {
                if (current == parentNode->m_pChild[LEFT] && parentNode->m_pChild[RIGHT]) {
                    this->m_pNode = parentNode->m_pChild[RIGHT];
                    return *this;
                }
                current = parentNode;
                parentNode = parentNode->parent;
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
        Node* parentNode = current->parent;

        if (!parentNode) { this->m_pNode = nullptr; return *this; }

        if (current == parentNode->m_pChild[LEFT] && parentNode->m_pChild[RIGHT]) current = deepest(parentNode->m_pChild[RIGHT]);
        else current = parentNode;

        this->m_pNode = current;
        return *this;
    }
};

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
protected:
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
    ~BinaryTree() { clear(); }

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

    using InorderForwardIterator   = InorderIterator<Myself, false>;
    using InorderBackwardIterator  = InorderIterator<Myself, true>;
    using PreorderForwardIterator  = PreorderIterator<Myself, false>;
    using PreorderBackwardIterator = PreorderIterator<Myself, true>;
    using PostorderForwardIterator = PostorderIterator<Myself, false>;
    using PostorderBackwardIterator = PostorderIterator<Myself, true>;

    friend InorderForwardIterator;   friend InorderBackwardIterator;
    friend PreorderForwardIterator;  friend PreorderBackwardIterator;
    friend PostorderForwardIterator; friend PostorderBackwardIterator;

    InorderForwardIterator begin() {
        Node* c = m_pRoot;
        while(c && c->m_pChild[0]) c = c->m_pChild[0];
        return InorderForwardIterator(this, c); }
    InorderForwardIterator end()   { return InorderForwardIterator(this, nullptr); }
    InorderForwardIterator begin() const {
        Node* c = m_pRoot;
        while(c && c->m_pChild[0]) c = c->m_pChild[0];
        return InorderForwardIterator(const_cast<Myself*>(this), c); }
    InorderForwardIterator end() const   { return InorderForwardIterator(const_cast<Myself*>(this), nullptr); }

    InorderBackwardIterator rbegin() {
        Node* c = m_pRoot;
        while(c && c->m_pChild[1]) c = c->m_pChild[1];
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
    //impresion
    void print_inorder() { for(auto it = begin(); it != end(); ++it) std::cout << *(it.getNode()) << " "; std::cout << std::endl; }
    void print_preorder() { for(auto it = begin_preorder(); it != end_preorder(); ++it) std::cout << *(it.getNode()) << " "; std::cout << std::endl; }
    void print_postorder() { for(auto it = begin_postorder(); it != end_postorder(); ++it) std::cout << *(it.getNode()) << " "; std::cout << std::endl; }
    //impresion inversa
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
            if(nextIt != Btn.end()) os << ",";
        }
        return os << "]";
    }

    friend std::istream& operator>>(std::istream& is, BinaryTree& Btn) {
        //std::unique_lock<std::shared_mutex> lock(Btn.m_mtx); real_list llama a insert
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
        pNode->parent = parent;
        return;
    }
    // internal_clone e internal_insert acceden de manera segura a los datos privados del nodo gracias a la relación de amistad
    auto branch = m_comp(pNode->m_data, data);
    internal_insert(pNode->m_pChild[branch], pNode, data, ref);
}

#endif // __BINARYTREE_H__
