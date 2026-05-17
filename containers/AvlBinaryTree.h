#include "BinaryTree.h"
#include <algorithm>
#include <mutex>
#include <shared_mutex>

// 1. Ahora el nodo recibe el tipo de altura por plantilla (por defecto int si se quiere, o configurado por el trait)
template<typename T, typename HeightType = T1>
struct AVLNode : BinaryTreeNodeBase<T, AVLNode<T, HeightType>>
{
    using Base = BinaryTreeNodeBase<T, AVLNode<T, HeightType>>;
    using value_type = T;
    HeightType m_height;
    AVLNode(T data, Ref ref): Base(data, ref),m_height(static_cast<HeightType>(1)){}
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

// 2. El Trait ahora define explícitamente las políticas de tipos
template<typename T>
struct AVLAscendingTrait : public BaseTrait<AVLNode<T, T1>, std::less<T>>
{
    // Definimos el tipo de altura dentro del trait para que el árbol lo herede
    using height_type = T1;
};

// 3. El árbol se adapta al Trait sin conocer tipos nativos fijos
template<typename Trait>
class AVLTree : public BinaryTree<Trait>
{
protected:
    using Node       = typename BinaryTree<Trait>::Node;
    using value_type = typename BinaryTree<Trait>::value_type;

    // Extraemos el tipo de altura directamente desde el Trait de manera abstracta
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
    // El árbol ahora calcula el balance usando la abstracción de 'height_type'
    height_type height(Node* node) const
    {
        return node ? node->getHeight() : static_cast<height_type>(0);
    }

    // El factor de balance también respeta el tipo abstracto
    height_type getBalance(Node* node) const
    {
        if (!node) return static_cast<height_type>(0);
        return height(node->m_pChild[0]) - height(node->m_pChild[1]);
    }

    void updateHeight(Node* node)
    {
        if (node) {
            // Se adapta al tipo definido sin asumir que es 'int'
            node->setHeight(static_cast<height_type>(1) + std::max(height(node->m_pChild[0]), height(node->m_pChild[1])));
        }
    }

    Node* rotate(Node* y, int branch)
    {
        int opposite = 1 - branch;
        Node* x = y->m_pChild[branch];
        Node* T2 = x->m_pChild[opposite];

        x->m_pChild[opposite] = y;
        y->m_pChild[branch] = T2;

        x->parent = y->parent;
        y->parent = x;
        if (T2) T2->parent = y;

        updateHeight(y);
        updateHeight(x);

        return x;
    }

    Node* internal_insert(Node* node, Node* parent, value_type data, Ref ref)
    {
        if (node == nullptr) {
            Node* newNode = new Node(data, ref);
            newNode->parent = parent;
            return newNode;
        }

        int branch = m_comp(node->m_data, data) ? 1 : 0;
        node->m_pChild[branch] = internal_insert(node->m_pChild[branch], node, data, ref);

        updateHeight(node);
        height_type balance = getBalance(node);

        // Constantes de desbalance abstractas
        height_type positive_threshold = static_cast<height_type>(1);
        height_type negative_threshold = static_cast<height_type>(-1);

        // Caso Izquierda - Izquierda
        if (balance > positive_threshold && !m_comp(node->m_pChild[0]->m_data, data)) {
            return rotate(node, 0);
        }

        // Caso Derecha - Derecha
        if (balance < negative_threshold && m_comp(node->m_pChild[1]->m_data, data)) {
            return rotate(node, 1);
        }

        // Caso Izquierda - Derecha
        if (balance > positive_threshold && m_comp(node->m_pChild[0]->m_data, data)) {
            node->m_pChild[0] = rotate(node->m_pChild[0], 1);
            return rotate(node, 0);
        }

        // Caso Derecha - Izquierda
        if (balance < negative_threshold && !m_comp(node->m_pChild[1]->m_data, data)) {
            node->m_pChild[1] = rotate(node->m_pChild[1], 0);
            return rotate(node, 1);
        }

        return node;
    }
};
