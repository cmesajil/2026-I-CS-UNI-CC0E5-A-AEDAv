#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include "BTreePage.h"

#define DEFAULT_BTREE_ORDER 3

template <typename Trait>
class BTree {
public:
    // Reemplazos de typedef tradicionales a "using" de C++ Moderno
    using Page           = CBTreePage<Trait>;
    using PagePtr        = Page*;
    using BTNode         = CBTreePage<Trait>;
    using ObjectInfo     = typename BTNode::ObjectInfo;
    using Entry          = typename Trait::Node;
    using Node           = Entry;
    using value_type     = typename Trait::value_type;

    using lpfnForEach2   = typename BTNode::lpfnForEach2;
    using lpfnForEach3   = typename BTNode::lpfnForEach3;
    using lpfnFirstThat2 = typename BTNode::lpfnFirstThat2;
    using lpfnFirstThat3 = typename BTNode::lpfnFirstThat3;

private:
    Page* m_root = nullptr;

protected:
    BTNode m_Root; // Firma heredada opcional
    size_t m_Height;
    size_t m_Order;
    size_t m_NumKeys;
    bool   m_Unique;

public:
    BTree(size_t order = DEFAULT_BTREE_ORDER, bool unique = true)
        : m_root(nullptr), m_Height(0), m_Order(order), m_NumKeys(0), m_Unique(unique) {}
    ~BTree() {}

    // Métodos clásicos preservados para futura implementación externa
    bool      Insert (const value_type key, const Ref ObjID) {
        if (!m_root) m_root = new Page();
        if (m_root->get_size() < Trait::max_keys) {
            m_root->get_item(m_root->get_size()) = Entry(key, ObjID);
            m_root->NumberOfKeys()++;
            m_NumKeys++;
        }
        return true;
    }
    bool      Remove (const value_type key, const Ref ObjID) { return false; }
    Ref       Search (const value_type key) { return 0; }
    size_t    size()     { return m_NumKeys; }
    size_t    height()   { return m_Height; }
    size_t    GetOrder() { return m_Order; }

    void      Print (ostream &os);

    // Soporte clásico con callbacks tradicionales de tus firmas anteriores
    void        ForEach(lpfnForEach2 lpfn, void *pExtra1);
    void        ForEach(lpfnForEach3 lpfn, void *pExtra1, void *pExtra2);
    ObjectInfo* FirstThat(lpfnFirstThat2 lpfn, void *pExtra1);
    ObjectInfo* FirstThat(lpfnFirstThat3 lpfn, void *pExtra1, void *pExtra2);

public:
    // --- ENVOLTURAS VARIÁDICAS MODERNAS CON PERFECT FORWARDING ---
    template <typename Func, typename... Args>
    void forEach(Func&& func, Args&&... args) {
        if (m_root) {
            m_root->forEach(0, std::forward<Func>(func), std::forward<Args>(args)...);
        }
    }

    template <typename Func, typename... Args>
    Entry* firstThat(Func&& func, Args&&... args) {
        if (m_root) {
            return m_root->firstThat(0, std::forward<Func>(func), std::forward<Args>(args)...);
        }
        return nullptr;
    }

    template <typename Func, typename... Args>
    void forEachPage(Func&& func, Args&&... args) {
        if (m_root) {
            m_root->forEachPage(0, std::forward<Func>(func), std::forward<Args>(args)...);
        }
    }

    // --- ENLACE CON EL ITERADOR (Soporta Range-based for loops) ---
    using iterator = BTreeForwardIterator<BTree<Trait>>;
    iterator begin() { return iterator(this, m_root); }
    iterator end()   { return iterator(this); }
};

#endif
