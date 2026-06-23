#ifndef CBTREEPAGE_H
#define CBTREEPAGE_H

#include <vector>
#include <iostream>
#include <assert.h>
#include <stack>
#include <future>
#include <utility>
#include "../types.h"
#include "traits.h"
#include "general_iterator.h"

template <typename Trait>
class BTree;

using namespace std;

enum bt_ErrorCode { bt_ok, bt_overflow, bt_underflow, bt_duplicate, bt_nofound, bt_rootmerged };

// =========================================================================
// ITERADOR DEL ÁRBOL B
// =========================================================================
template <typename Container>
class BTreeForwardIterator : public general_iterator<Container, BTreeForwardIterator<Container>> {
public:
    using MySelf   = BTreeForwardIterator<Container>;
    using Parent   = general_iterator<Container, MySelf>;
    using PagePtr  = typename Container::PagePtr;
    using Parent::Parent;

private:
    std::stack<std::pair<PagePtr, size_t>> m_stack;

    void push_left(PagePtr page) {
        while (page != nullptr) {
            m_stack.push({page, 0});
            if (page->is_leaf()) break;
            page = page->get_child(0);
        }
    }

public:
    BTreeForwardIterator(Container* container, PagePtr root) : Parent(container, nullptr) {
        if (root != nullptr) {
            push_left(root);
            this->m_pNode = &(m_stack.top().first->get_item(m_stack.top().second));
        }
    }

    BTreeForwardIterator(Container* container) : Parent(container, nullptr) {}

    MySelf& operator++() {
        if (m_stack.empty()) {
            this->m_pNode = nullptr;
            return *this;
        }

        auto& top = m_stack.top();
        PagePtr page = top.first;
        size_t& index = top.second;

        if (!page->is_leaf()) {
            index++;
            push_left(page->get_child(index));
        } else {
            index++;
            while (!m_stack.empty() && m_stack.top().second >= m_stack.top().first->get_size()) {
                m_stack.pop();
            }
        }

        if (!m_stack.empty()) {
            this->m_pNode = &(m_stack.top().first->get_item(m_stack.top().second));
        } else {
            this->m_pNode = nullptr;
        }

        return *this;
    }
};

// =========================================================================
// ELEMENTO DEL NODO (BTreeNodeItem)
// =========================================================================
template <typename T>
class BTreeNodeItem {
private:
    T      m_data;
    Ref    m_ref;
    size_t UseCounter;
public:
    using value_type = T;

    BTreeNodeItem(const value_type &_data, Ref _ref) : m_data(_data), m_ref(_ref), UseCounter(0) {}
    BTreeNodeItem() : UseCounter(0) {}

    operator value_type() const { return m_data; }

    const value_type& GetData() const { return m_data; }
    value_type& getDataRef() { return m_data; }
    value_type* GetDataPtr() { return &m_data; }
    void SetData(const value_type& data) { m_data = data; }

    Ref GetRef() const { return m_ref; }
    const Ref* GetRefPtr() const { return &m_ref; }
    void SetRef(Ref ref) { m_ref = ref; }

    size_t GetUseCounter() const { return UseCounter; }
    void SetUseCounter(size_t counter) { UseCounter = counter; }
    void IncrementUseCounter() { ++UseCounter; }
};

// Traits adaptados
template <typename _Node, size_t MaxKeys, typename _Comp, bool Unique = true>
struct BTreeTrait : public BaseTrait<_Node, _Comp> {
    static constexpr size_t max_keys = MaxKeys;
    static constexpr bool is_unique  = Unique;
    using Node = _Node;
    using value_type = typename _Node::value_type;
};

template <typename T, typename Comp = std::less<T>>
struct Tree23TraitAscending : public BTreeTrait<BTreeNodeItem<T>, 2, Comp, true> {};

template <typename T, typename Comp = std::greater<T>>
struct Tree23TraitDescending : public BTreeTrait<BTreeNodeItem<T>, 2, Comp, true> {};

template <typename T, typename Comp = std::less<T>>
struct Tree34TraitMulti : public BTreeTrait<BTreeNodeItem<T>, 3, Comp, false> {};

// =========================================================================
// CLASE PÁGINA (CBTreePage)
// =========================================================================
template <typename Trait>
class CBTreePage {
    friend class BTree<Trait>;

    using BTPage     = CBTreePage<Trait>;
    using ObjectInfo = typename Trait::Node;

public:
    using value_type = typename Trait::value_type;
    using Node       = CBTreePage<Trait>;

    // Punteros a funciones antiguas heredados
    using lpfnForEach2 = void (*)(ObjectInfo &info, size_t level, void *pExtra1);
    using lpfnForEach3 = void (*)(ObjectInfo &info, size_t level, void *pExtra1, void *pExtra2);
    using lpfnFirstThat2 = ObjectInfo* (*)(ObjectInfo &info, size_t level, void *pExtra1);
    using lpfnFirstThat3 = ObjectInfo* (*)(ObjectInfo &info, size_t level, void *pExtra1, void *pExtra2);

public:
    CBTreePage() : m_MaxKeysForChilds(Trait::max_keys), m_isRoot(false), m_KeyCount(0) {
        m_Keys.resize(Trait::max_keys + 1);
        m_SubPages.resize(Trait::max_keys + 2, nullptr);
    }
    virtual ~CBTreePage() {}

    bt_ErrorCode Insert (const value_type &m_data, const Ref m_ref);
    bt_ErrorCode Remove (const value_type &m_data, const Ref m_ref);
    bool         Search (const value_type &m_data, size_t &m_ref);
    void         Print  (ostream &os);

    // Métodos antiguos que usarán punteros a función tradicionales
    void         ForEach(lpfnForEach2 lpfn, size_t level, void *pExtra1);
    void         ForEach(lpfnForEach3 lpfn, size_t level, void *pExtra1, void *pExtra2);
    ObjectInfo* FirstThat(lpfnFirstThat2 lpfn, size_t level, void *pExtra1);
    ObjectInfo* FirstThat(lpfnFirstThat3 lpfn, size_t level, void *pExtra1, void *pExtra2);

protected:
    size_t             m_MaxKeysForChilds;
    bool               m_isRoot;
    vector<ObjectInfo> m_Keys;
    vector<BTPage *>   m_SubPages;
    size_t             m_KeyCount;

    void  Create();
    void  Reset ();
    void  Destroy () { Reset(); delete this; }
    void  clear ();

    bool  Redistribute1   (size_t &pos);
    bool  Redistribute2   (size_t pos);
    void  RedistributeR2L (size_t pos);
    void  RedistributeL2R (size_t pos);

    bool  TreatUnderflow  (size_t &pos) { return Redistribute1(pos) || Redistribute2(pos); }

    bt_ErrorCode Merge  (size_t pos);
    bt_ErrorCode MergeRoot ();
    void  SplitChild (size_t pos);

    ObjectInfo &GetFirstObjectInfo();

    bool Overflow()  { return m_KeyCount > Trait::max_keys; }
    bool Underflow() { return m_KeyCount < MinNumberOfKeys(); }
    bool IsFull()    { return m_KeyCount >= Trait::max_keys; }

    size_t  MinNumberOfKeys()  { return static_cast<size_t>(2 * Trait::max_keys / 3.0); }
    size_t  GetFreeCells()     { return Trait::max_keys - m_KeyCount; }
    size_t& NumberOfKeys()     { return m_KeyCount; }
    size_t  GetNumberOfKeys()  { return m_KeyCount; }

    bool IsRoot()  { return m_MaxKeysForChilds != Trait::max_keys; }
    void SetMaxKeysForChilds(size_t orderforchilds) { m_MaxKeysForChilds = orderforchilds; }

    size_t GetFreeCellsOnLeft(size_t pos);
    size_t GetFreeCellsOnRight(size_t pos);

private:
    bool SplitRoot();
    void SplitPageInto3(vector<ObjectInfo> & tmpKeys, vector<BTPage *> & SubPages,
                        BTPage *& pChild1, BTPage *& pChild2, BTPage *& pChild3,
                        ObjectInfo & oi1, ObjectInfo & oi2);
    void MovePage(BTPage * pChildPage, vector<ObjectInfo> & tmpKeys, vector<BTPage *> & tmpSubPages);

public:
    // Auxiliares requeridos por el iterador
    bool is_leaf() const { return m_SubPages.empty() || m_SubPages[0] == nullptr; }
    BTPage* get_child(size_t idx) { return m_SubPages[idx]; }
    ObjectInfo& get_item(size_t idx) { return m_Keys[idx]; }
    size_t get_size() const { return m_KeyCount; }

    // --- VARIÁDICOS COMPLETOS ---
    template <typename Func, typename... Args>
    void forEach(size_t level, Func func, Args&&... args) {
        std::vector<std::future<void>> futures;
        for (size_t i = 0; i < m_KeyCount; ++i) {
            if (m_SubPages[i]) {
                futures.push_back(std::async(std::launch::async, [this, i, level, &func, &args...]() {
                    m_SubPages[i]->forEach(level + 1, func, std::forward<Args>(args)...);
                }));
            }
            func(m_Keys[i], level, std::forward<Args>(args)...);
        }
        if (m_SubPages[m_KeyCount]) {
            futures.push_back(std::async(std::launch::async, [this, level, &func, &args...]() {
                m_SubPages[m_KeyCount]->forEach(level + 1, func, std::forward<Args>(args)...);
            }));
        }
        for (auto& f : futures) {
            f.get();
        }
    }

    template <typename Func, typename... Args>
    ObjectInfo* firstThat(size_t level, Func func, Args&&... args) {
        for (size_t i = 0; i < m_KeyCount; ++i) {
            if (m_SubPages[i])
                if (ObjectInfo* found = m_SubPages[i]->firstThat(level + 1, func, std::forward<Args>(args)...))
                    return found;
            if (func(m_Keys[i], level, std::forward<Args>(args)...)) return &m_Keys[i];
        }
        if (m_SubPages[m_KeyCount])
            return m_SubPages[m_KeyCount]->firstThat(level + 1, func, std::forward<Args>(args)...);
        return nullptr;
    }

    template <typename Func, typename... Args>
    void forEachPage(size_t level, Func func, Args&&... args) {
        func(m_KeyCount, level, std::forward<Args>(args)...);
        for (size_t i = 0; i <= m_KeyCount; ++i)
            if (m_SubPages[i]) m_SubPages[i]->forEachPage(level + 1, func, std::forward<Args>(args)...);
    }
};

#endif
