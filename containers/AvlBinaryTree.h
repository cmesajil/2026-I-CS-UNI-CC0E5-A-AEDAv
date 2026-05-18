#ifndef __AVLABINARYTREE_H__
#define __AVLABINARYTREE_H__

#include "BinaryTree.h"
#include <algorithm>
#include <mutex>
#include <shared_mutex>

// 1. El nodo recibe el tipo de altura por plantilla
template<typename T, typename HeightType = T1>
struct AVLNode : public BinaryTreeNodeBase<T, AVLNode<T, HeightType>>
{
    using Base = BinaryTreeNodeBase<T, AVLNode<T, HeightType>>;
    using value_type = T;
    HeightType m_height;

    AVLNode(T data, Ref ref) : Base(data, ref), m_height(static_cast<HeightType>(1)) {}

    HeightType getHeight() const { return m_height; }
    void setHeight(HeightType h) { m_height = h; }
};

template<typename T, typename H>
ostream& operator<<(ostream& os, const AVLNode<T,H>& node)
{
    return os
        << "("
        << node.getData()
        << ", "
        << node.getRef()
        << ", h="
        << node.getHeight()
        << ")";
}

// 2. El Trait define las políticas de tipos
template<typename T>
struct AVLAscendingTrait : public BaseTrait<AVLNode<T, T1>, std::less<T>>
{
    using height_type = T1;
};



// 3. El árbol AVL adaptado respetando el encapsulamiento privado de la base
template<typename Trait>
class AVLTree : public BinaryTree<Trait>
{
protected:
    using Node       = typename BinaryTree<Trait>::Node;
    using value_type = typename BinaryTree<Trait>::value_type;
    using height_type = typename Trait::height_type;

    using BinaryTree<Trait>::m_pRoot;
    using BinaryTree<Trait>::m_comp;

public:
    AVLTree() : BinaryTree<Trait>() {}

    void insert(value_type data, Ref ref)
    {
        std::unique_lock<std::shared_mutex> lock(this->m_mtx);
        m_pRoot = internal_insert(m_pRoot, nullptr, data, ref);
    }

private:
    // Usa el método público getChild() de la base de forma segura
    height_type height(Node* node) const
    {
        return node ? node->getHeight() : static_cast<height_type>(0);
    }

    // Usa getChild() para calcular el factor de balance de manera abstracta
    height_type getBalance(Node* node) const
    {
        if (!node) return static_cast<height_type>(0);
        return height(node->getChild(0)) - height(node->getChild(1));
    }

    // Actualiza la altura llamando a getChild()
    void updateHeight(Node* node)
    {
        if (node) {
            node->setHeight(static_cast<height_type>(1) + std::max(height(node->getChild(0)), height(node->getChild(1))));
        }
    }

    // Rotaciones adaptadas completamente a Getters y Setters
    Node* rotate(Node* y, int branch)
    {
        int opposite = 1 - branch;
        Node* x = y->getChild(branch);
        Node* T2 = x->getChild(opposite);

        x->setChild(opposite, y);
        y->setChild(branch, T2);

        x->setParent(y->getParent());
        y->setParent(x);
        if (T2) T2->setParent(y);

        updateHeight(y);
        updateHeight(x);

        return x;
    }

    Node* internal_insert(Node* node, Node* parent, value_type data, Ref ref)
    {
        if (node == nullptr) {
            Node* newNode = new Node(data, ref);
            newNode->setParent(parent); // <-- Modificado a setter público
            return newNode;
        }

        // Usamos getData() para extraer el valor encapsulado
        int branch = m_comp(node->getData(), data) ? 1 : 0;

        Node* nextChild = internal_insert(node->getChild(branch), node, data, ref);
        node->setChild(branch, nextChild); // <-- Modificado a setter público

        updateHeight(node);
        height_type balance = getBalance(node);

        height_type positive_threshold = static_cast<height_type>(1);
        height_type negative_threshold = static_cast<height_type>(-1);

        // Caso Izquierda - Izquierda
        if (balance > positive_threshold && !m_comp(node->getChild(0)->getData(), data)) {
            return rotate(node, 0);
        }

        // Caso Derecha - Derecha
        if (balance < negative_threshold && m_comp(node->getChild(1)->getData(), data)) {
            return rotate(node, 1);
        }

        // Caso Izquierda - Derecha
        if (balance > positive_threshold && m_comp(node->getChild(0)->getData(), data)) {
            node->setChild(0, rotate(node->getChild(0), 1));
            return rotate(node, 0);
        }

        // Caso Derecha - Izquierda
        if (balance < negative_threshold && !m_comp(node->getChild(1)->getData(), data)) {
            node->setChild(1, rotate(node->getChild(1), 0));
            return rotate(node, 1);
        }

        return node;
    }
};

#endif // __AVLABINARYTREE_H__
