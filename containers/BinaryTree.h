#ifndef __BINARYTREE_H__
#define __BINARYTREE_H__
#include <iostream>
#include <cstddef>
#include <string>
#include <fstream>
#include <thread>
#include "../types.h"
using namespace std;


template<typename T>
struct BinaryTreeNode{
    T m_data;
    Ref m_ref;
    BinaryTreeNode *m_pChild[2];
    public:
        using value_type = T;
        BinaryTreeNode(T data) : m_data(data), m_pChild{nullptr, nullptr} {}
};

template <typename T>
struct AscendingBinaryTreeTrait : public BaseTrait<BinaryTreeNode<T>, std::less<T>> {};

template <typename T>
struct DescendingBinaryTreeTrait : public BaseTrait<BinaryTreeNode<T>, std::greater<T>> {};

//usar iteradores
template<typename Trait>
class BinaryTree{
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp    = typename Trait::Comp;
    private:
        Node    *m_pRoot;
        Comp  m_comp;
    public:
        BinaryTree() : m_pRoot(nullptr) {}
        ~BinaryTree() {}

protected:
        mutable shared_mutex m_mtx;
        void internal_insert(Node* &pNode, value_type data, Ref ref);

public:
        void insert(value_type data, Ref ref){
            internal_insert(m_pRoot, data, ref);
        }
        void print(){
            print(std::cout, m_pRoot);
        }
        void print(std::ostream& os, Node* node) const {

            if(node){

                print(os, node->m_pChild[0]);

                os << node->m_data << " ";

                print(os, node->m_pChild[1]);
            }
        }

        friend std::ostream& operator<<(std::ostream& os,
                                        const BinaryTree& Btn)
        {
            std::shared_lock<std::shared_mutex> lock(Btn.m_mtx);

            os << "[ ";

            Btn.print(os, Btn.m_pRoot);

            os << "]";

            return os;
        }

        friend std::istream& operator>>(std::istream& is, BinaryTree& Btn) {
                std::unique_lock<std::shared_mutex> lock(Btn.m_mtx);
                return read_list(is, Btn);
            }
};

template<typename Trait>
void BinaryTree<Trait>::internal_insert(Node* &pNode, value_type data, Ref ref){
    if( pNode == nullptr ){
        pNode = new Node(data);
        return;
    }
    auto branch = m_comp(pNode->m_data, data);
    internal_insert(pNode->m_pChild[branch], data, ref);
}
#endif // __BINARYTREE_H__
