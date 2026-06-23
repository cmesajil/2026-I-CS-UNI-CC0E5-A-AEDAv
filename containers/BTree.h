#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include "traits.h"
#include "BTreePage.h"

template <typename _Node, size_t MaxKeys, typename _Comp, bool Unique = true>
struct BTreeTrait : public BaseTrait<_Node, _Comp> {
    static constexpr size_t max_keys = MaxKeys;
    static constexpr bool is_unique = Unique;

    using Node       = _Node;
    using value_type = typename _Node::value_type;
};

template <typename T, typename Comp = std::less<T>>
struct Tree23TraitAscending :
    public BTreeTrait<BTreeNodeItem<T>, 2, Comp, true> {};

template <typename T, typename Comp = std::greater<T>>
struct Tree23TraitDescending :
    public BTreeTrait<BTreeNodeItem<T>, 2, Comp, true> {};

template <typename T, typename Comp = std::less<T>>
struct Tree34TraitMulti :
    public BTreeTrait<BTreeNodeItem<T>, 3, Comp, false> {};

template <typename Trait>
class BTree {
public:
    using Page       = CBTreePage<Trait>;
    using PagePtr    = Page*;
    using ObjectInfo = typename Page::ObjectInfo;
    using Node = typename Page::ObjectInfo;

    using node_type  = typename Trait::Node;
    using value_type = typename Trait::value_type;
    using comparator = typename Trait::Comp;


private:
    PagePtr m_root = nullptr;
    comparator m_comp{};

protected:
    size_t m_Height = 0;
    size_t m_NumKeys = 0;

public:
    BTree() = default;

    ~BTree() {
        if (m_root)
            m_root->Destroy();
    }

    bool Insert(const value_type key, const Ref ObjID);
    bool Remove(const value_type key, const Ref ObjID);

    Ref Search(const value_type key);

    size_t size() const {
        return m_NumKeys;
    }

    size_t height() const {
        return m_Height;
    }

    constexpr size_t GetOrder() const {
        return Trait::max_keys;
    }

    constexpr bool IsUnique() const {
        return Trait::is_unique;
    }

    template <typename Func, typename... Args>
    void forEach(Func&& func, Args&&... args) {
        if (m_root) {
            m_root->forEach(
                0,
                std::forward<Func>(func),
                std::forward<Args>(args)...);
        }
    }

    template <typename Func, typename... Args>
    ObjectInfo* firstThat(Func&& func, Args&&... args) {
        if (m_root) {
            return m_root->firstThat(
                0,
                std::forward<Func>(func),
                std::forward<Args>(args)...);
        }
        return nullptr;
    }

    using iterator = BTreeForwardIterator<BTree<Trait>>;

    iterator begin() {
        return iterator(this, m_root);
    }

    iterator end() {
        return iterator(this);
    }
};

template <typename Trait>
bool BTree<Trait>::Insert(const value_type key, const Ref ObjID)
{
    if (m_root == nullptr) {
        m_root = new Page();
        m_Height = 1;
    }

    bt_ErrorCode error = m_root->Insert(key, ObjID);

    if (error == bt_duplicate)
        return false;

    ++m_NumKeys;

    if (error == bt_overflow) {
        m_root->SplitRoot();
        ++m_Height;
    }

    return true;
}

#endif
