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

//iteradores
template<typename Container>
class InorderIterator
    : public general_iterator<Container,
                              InorderIterator<Container>>
{
public:

    using MySelf = InorderIterator<Container>;

    using Parent =
        general_iterator<Container, MySelf>;

    using Node = typename Container::Node;

    using Parent::Parent;

    MySelf& operator++()
    {
        if(!this->m_pNode)
            return *this;

        Node* current = this->m_pNode;

        // Caso 1: tiene right child
        if(current->m_pChild[1]){

            current = current->m_pChild[1];

            while(current->m_pChild[0])
                current = current->m_pChild[0];
        }
        else{

            // subir usando parent
            Node* parent = current->parent;

            while(parent &&
                  current == parent->m_pChild[1])
            {
                current = parent;
                parent = parent->parent;
            }

            current = parent;
        }

        this->m_pNode = current;

        return *this;
    }
};

template<typename Container>
class PreorderIterator
    : public general_iterator<
            Container,
            PreorderIterator<Container>>
{
public:

    using MySelf = PreorderIterator<Container>;

    using Parent =
        general_iterator<Container, MySelf>;

    using Node = typename Container::Node;

    using Parent::Parent;

    MySelf& operator++()
    {
        if(!this->m_pNode)
            return *this;

        Node* current = this->m_pNode;

        // 1. izquierda
        if(current->m_pChild[0]){

            current = current->m_pChild[0];
        }

        // 2. derecha
        else if(current->m_pChild[1]){

            current = current->m_pChild[1];
        }

        // 3. subir
        else{

            Node* parent = current->parent;

            while(parent){

                // si venimos de izquierda
                // y hay derecha pendiente
                if(current == parent->m_pChild[0] &&
                   parent->m_pChild[1])
                {
                    current = parent->m_pChild[1];

                    this->m_pNode = current;

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

template<typename Container>
class PostorderIterator
    : public general_iterator<
            Container,
            PostorderIterator<Container>>
{
public:

    using MySelf = PostorderIterator<Container>;

    using Parent =
        general_iterator<Container, MySelf>;

    using Node = typename Container::Node;

    using Parent::Parent;

    static Node* deepest(Node* node)
    {
        while(node){

            if(node->m_pChild[0])
                node = node->m_pChild[0];

            else if(node->m_pChild[1])
                node = node->m_pChild[1];

            else
                break;
        }

        return node;
    }

    MySelf& operator++()
    {
        if(!this->m_pNode)
            return *this;

        Node* current = this->m_pNode;

        Node* parent = current->parent;

        // root terminado
        if(!parent){

            this->m_pNode = nullptr;

            return *this;
        }

        // venimos de izquierda
        // y existe derecha pendiente
        if(current == parent->m_pChild[0] &&
           parent->m_pChild[1])
        {
            current = deepest(parent->m_pChild[1]);
        }
        else{

            // subir al padre
            current = parent;
        }

        this->m_pNode = current;

        return *this;
    }
};

template<typename T>
struct BinaryTreeNode{
    T m_data;
    Ref m_ref;
    BinaryTreeNode *parent; //nos permite navegar hacia arriba
    BinaryTreeNode *m_pChild[2];
    public:
        using value_type = T;
        BinaryTreeNode(T data, Ref ref)
            : m_data(data),
              m_ref(ref),
              parent(nullptr),
              m_pChild{nullptr, nullptr} {}

        BinaryTreeNode* getChild(int index) const { return m_pChild[index]; }
        BinaryTreeNode* getParent() const { return parent; }
        T      getData() const { return m_data; }
        T&     getDataRef()    { return m_data; }
        void   setData(T data) { m_data = data; }
        Ref    getRef() const  { return m_ref; }
        void   setRef(Ref ref) { m_ref = ref; }


};

template <typename T>
ostream& operator<<(ostream& os, BinaryTreeNode<T>& node){
    return os << "(" << node.m_data << ", " << node.getRef() << ")";
}

template <typename T>
struct AscendingBinaryTreeTrait : public BaseTrait<BinaryTreeNode<T>, std::less<T>> {};

template <typename T>
struct DescendingBinaryTreeTrait : public BaseTrait<BinaryTreeNode<T>, std::greater<T>> {};

//usar iteradores
template<typename Trait>
class BinaryTree{
    public:
        using value_type = typename Trait::value_type;
        using Node       = typename Trait::Node;
        using Comp    = typename Trait::Comp;
        using Myself = BinaryTree<Trait>;
    private:
        Node    *m_pRoot;
        Comp  m_comp;
    public:
        BinaryTree() : m_pRoot(nullptr) {}
        ~BinaryTree() {}

protected:
        mutable shared_mutex m_mtx;
        void internal_insert(Node*& pNode,
                             Node* parent,
                             value_type data,
                             Ref ref);

public:
        using forward_InorderIterator = InorderIterator<Myself>;
        friend forward_InorderIterator;
        using const_iterator = InorderIterator<const Myself>;
        friend const_iterator;
        using forward_PreOrderIterator = PreorderIterator<Myself>;
        friend forward_PreOrderIterator;
        using forward_PostOrderIterator = PostorderIterator<Myself>;
        friend forward_PostOrderIterator;

        forward_InorderIterator begin() { Node* current = m_pRoot;

            while(current && current->m_pChild[0])
                current = current->m_pChild[0];

            return forward_InorderIterator(this, current); }
        forward_InorderIterator end()   { return InorderIterator(this, nullptr); }
        const_iterator begin() const { Node* current = m_pRoot;

            while(current && current->m_pChild[0])
                current = current->m_pChild[0];

            return const_iterator(this, current); }
        const_iterator end()   const { return const_iterator(this, nullptr); }

        //preorder
        forward_PreOrderIterator begin_preorder() { Node* current = m_pRoot;
            return forward_PreOrderIterator(this, current); }
        forward_PreOrderIterator end_preorder()   { return forward_PreOrderIterator(this, nullptr); }

        //postorder
        forward_PostOrderIterator begin_postorder() { Node* current = m_pRoot;

            while(current){

                if(current->m_pChild[0])
                    current = current->m_pChild[0];

                else if(current->m_pChild[1])
                    current = current->m_pChild[1];

                else
                    break;
            }

            return forward_PostOrderIterator(this, current); }
        forward_PostOrderIterator end_postorder()   { return forward_PostOrderIterator(this, nullptr); }


        // ForEach
        template <typename Func, typename... Args>
        void ForEach(Func func, Args &&... args) {
            unique_lock<shared_mutex> lock(m_mtx);
            if (m_pRoot == nullptr) return;
            for(auto& item : *this) {
                func(item, std::forward<Args>(args)...);
            }
        }

        void insert(value_type data, Ref ref){
            internal_insert(m_pRoot, nullptr, data, ref);
        }

        void print_inorder() {

            for(auto it = begin();
                it != end();
                ++it)
            {
                std::cout
                    << *(it.getNode())
                    << " ";
            }
            std::cout << std::endl;
        }

        void print_preorder()
        {
            for(auto it = begin_preorder();
                it != end_preorder();
                ++it)
            {
                std::cout
                    << *(it.getNode())
                    << " ";
            }

            std::cout << std::endl;
        }

        void print_postorder() {

            for(auto it = begin_postorder();
                it != end_postorder();
                ++it)
            {
                std::cout
                    << *(it.getNode())
                    << " ";
            }
            std::cout << std::endl;
        }

        friend std::ostream& operator<<(std::ostream& os,
                                        const BinaryTree& Btn)
        {
            std::shared_lock<std::shared_mutex> lock(Btn.m_mtx);

            os << "[ ";

            for(auto it = Btn.begin();
                it != Btn.end();
                ++it){

                cout << "("
                     << it->getData()
                     << ", "
                     << it->getRef()
                     << ") ";
            }

            os << "]";

            return os;
        }

        friend std::istream& operator>>(std::istream& is, BinaryTree& Btn) {
                std::unique_lock<std::shared_mutex> lock(Btn.m_mtx);
                return read_list(is, Btn);
            }
};

template<typename Trait>
void BinaryTree<Trait>::internal_insert(
    Node*& pNode,
    Node* parent,
    value_type data,
    Ref ref)
{
    if(pNode == nullptr){

        pNode = new Node(data,ref);

        pNode->parent = parent;

        return;
    }

    auto branch = m_comp(pNode->m_data, data);

    internal_insert(
        pNode->m_pChild[branch],
        pNode,
        data,
        ref);
}
#endif // __BINARYTREE_H__
