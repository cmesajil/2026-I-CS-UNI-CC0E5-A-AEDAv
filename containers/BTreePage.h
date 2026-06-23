//CBTreePage.h

/*************************
#ifndef BTPage_H
#define BTPage_H
***************************/
#ifndef CBTreePage_H
#define CBTreePage_H
#include <vector>
#include <iostream>
#include <assert.h>
#include "../types.h"
#include "traits.h"

template <typename Trait>
class BTree;

using namespace std;
enum bt_ErrorCode {bt_ok, bt_overflow, bt_underflow, bt_duplicate, bt_nofound, bt_rootmerged};


#pragma once
#include "general_iterator.h"
#include <stack>
#include <utility>

template <typename Container>
class BTreeForwardIterator
    : public general_iterator<Container, BTreeForwardIterator<Container>> {

public:
    using MySelf = BTreeForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent; // Hereda constructores

    // Tipos internos del árbol extraídos a través de Container
    using PagePtr = typename Container::PagePtr;

private:
    // El "secreto" del árbol B: Necesitamos la pila para recordar los niveles superiores
    std::stack<std::pair<PagePtr, size_t>> m_stack;

    void push_left(PagePtr page) {
        while (page != nullptr) {
            m_stack.push({page, 0});
            if (page->is_leaf()) break;
            page = page->get_child(0);
        }
    }

public:
    // Constructor personalizado para inicializar la pila desde el nodo raíz
    BTreeForwardIterator(Container* container, PagePtr root)
        : Parent(container, nullptr) { // m_pNode se manejará dinámicamente o apuntará al ítem actual
        if (root != nullptr) {
            push_left(root);
            // Sincronizamos m_pNode de la clase base con el elemento actual en el tope de la pila
            this->m_pNode = &(m_stack.top().first->get_item(m_stack.top().second));
        }
    }

    // Constructor para el iterador final (end)
    BTreeForwardIterator(Container* container) : Parent(container, nullptr) {}

    // Sobrescritura del operador de avance para navegar la jerarquía del Árbol B
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

        // Actualizamos el puntero m_pNode de la clase padre general_iterator
        if (!m_stack.empty()) {
            this->m_pNode = &(m_stack.top().first->get_item(m_stack.top().second));
        } else {
            this->m_pNode = nullptr; // Llegamos al final (end)
        }

        return *this;
    }
};

// 1. Elemento del nodo
// 1. El elemento sigue siendo completamente limpio y agnóstico al orden
template <typename T>
class BTreeNodeItem {
private:
    T      m_data;
    Ref    m_ref;
    size_t UseCounter;
public:
    using value_type = T;

    public:
        BTreeNodeItem(const value_type &_data, Ref _ref)
               : m_data(_data), m_ref(_ref), UseCounter(0) {}
        BTreeNodeItem() : UseCounter(0) {}

        // Mantengo tu operador de conversión por compatibilidad
        operator value_type() const { return m_data; }

        // --- GETTERS & SETTERS PARA m_data ---

        // Getter por referencia constante (Evita copias, ideal para lectura masiva)
        const value_type& GetData() const { return m_data; }

        // Getter que retorna un puntero modificable (Útil si necesitas mutar m_data in-place)
        value_type* GetDataPtr() { return &m_data; }

        // Setter usando const& para máxima eficiencia al pasar objetos grandes
        void SetData(const value_type& data) { m_data = data; }


        // --- GETTERS & SETTERS PARA m_ref ---

        // Getter de Ref (Suele ser un tipo primitivo/puntero, se pasa por valor)
        Ref GetRef() const { return m_ref; }


        const Ref* GetRefPtr() const { return &m_ref; }

        void SetRef(Ref ref) { m_ref = ref; }


        // --- GETTERS & SETTERS PARA UseCounter ---

        size_t GetUseCounter() const { return UseCounter; }

        void SetUseCounter(size_t counter) { UseCounter = counter; }

        // Método de utilidad clásico para contadores: incrementar de a 1
        void IncrementUseCounter() { ++UseCounter; }
};

// Trait genérico y flexible para el Árbol B
template <typename _Node, size_t MaxKeys, typename _Comp, bool Unique = true>
struct BTreeTrait : public BaseTrait<_Node, _Comp> {
    static constexpr size_t max_keys = MaxKeys;
    static constexpr bool is_unique  = Unique;
};

// Árbol 2-3 Ascendente (Por defecto usa less)
template <typename T, typename Comp = std::less<T>>
struct Tree23TraitAscending : public BTreeTrait<BTreeNodeItem<T>, 2, Comp, true> {};

// Árbol 2-3 Descendente (Por defecto usa greater)
template <typename T, typename Comp = std::greater<T>>
struct Tree23TraitDescending : public BTreeTrait<BTreeNodeItem<T>, 2, Comp, true> {};

// Árbol 3-4 de búsqueda no única (Duplicados permitidos)
template <typename T, typename Comp = std::less<T>>
struct Tree34TraitMulti : public BTreeTrait<BTreeNodeItem<T>, 3, Comp, false> {};

// 3. Clase Principal de Página
template <typename Trait>
class CBTreePage
{
        friend class BTree<Trait>;

        using BTPage     = CBTreePage<Trait>;
        using ObjectInfo = BTreeNodeItem<Trait>;

        using lpfnForEach2 = void (*)(ObjectInfo &info, size_t level, void *pExtra1);
        using lpfnForEach3 = void (*)(ObjectInfo &info, size_t level, void *pExtra1, void *pExtra2);

        using lpfnFirstThat2 = ObjectInfo* (*)(ObjectInfo &info, size_t level, void *pExtra1);
        using lpfnFirstThat3 = ObjectInfo* (*)(ObjectInfo &info, size_t level, void *pExtra1, void *pExtra2);

public:
        using value_type = typename Trait::value_type;
        using Node       = CBTreePage<Trait>;

public:
        // El constructor ahora está limpio, no necesita parámetros de configuración dinámica
        CBTreePage();
        virtual ~CBTreePage();

        bt_ErrorCode    Insert (const value_type &m_data, const Ref m_ref);
        bt_ErrorCode    Remove (const value_type &m_data, const Ref m_ref);
        bool            Search (const value_type &m_data, size_t &m_ref);
        void            Print  (ostream &os);
        void            ForEach(lpfnForEach2 lpfn, size_t level, void *pExtra1);
        void            ForEach(lpfnForEach3 lpfn, size_t level, void *pExtra1, void *pExtra2);
        ObjectInfo* FirstThat(lpfnFirstThat2 lpfn, size_t level, void *pExtra1);
        ObjectInfo* FirstThat(lpfnFirstThat3 lpfn, size_t level, void *pExtra1, void *pExtra2);

protected:
        // m_MinKeys, m_MaxKeys y m_Unique ya no son variables miembro. Ahorramos memoria por nodo.
        size_t  m_MaxKeysForChilds;
        bool    m_isRoot;
        vector<ObjectInfo> m_Keys;
        vector<BTPage *>   m_SubPages;
        size_t  m_KeyCount;

        void  Create();
        void  Reset ();
        void  Destroy () { Reset(); delete this; }
        void  clear ();

        bool  Redistribute1   (size_t &pos);
        bool  Redistribute2   (size_t pos);
        void  RedistributeR2L (size_t pos);
        void  RedistributeL2R (size_t pos);

        bool  TreatUnderflow  (size_t &pos)
        {       return Redistribute1(pos) || Redistribute2(pos); }

        bt_ErrorCode  Merge  (size_t pos);
        bt_ErrorCode  MergeRoot ();
        void  SplitChild (size_t pos);

        ObjectInfo &GetFirstObjectInfo();

        // Accedemos a las propiedades directamente desde el Trait en tiempo de compilación
        bool Overflow()  { return m_KeyCount > Trait::max_keys; }
        bool Underflow() { return m_KeyCount < MinNumberOfKeys(); }
        bool IsFull()    { return m_KeyCount >= Trait::max_keys; }

        size_t  MinNumberOfKeys()  { return static_cast<size_t>(2 * Trait::max_keys / 3.0); }
        size_t  GetFreeCells()     { return Trait::max_keys - m_KeyCount; }
        size_t& NumberOfKeys()     { return m_KeyCount; }
        size_t  GetNumberOfKeys()  { return m_KeyCount; }

        bool IsRoot()  { return m_MaxKeysForChilds != Trait::max_keys; }
        void SetMaxKeysForChilds(size_t orderforchilds)
        {
                m_MaxKeysForChilds = orderforchilds;
        }

        size_t GetFreeCellsOnLeft(size_t pos);
        size_t GetFreeCellsOnRight(size_t pos);

private:
        bool SplitRoot();
        void SplitPageInto3(vector<ObjectInfo>   & tmpKeys,
                               vector<BTPage *>  & SubPages,
                               BTPage           *& pChild1,
                               BTPage           *& pChild2,
                               BTPage           *& pChild3,
                               ObjectInfo        & oi1,
                               ObjectInfo        & oi2);
        void MovePage(BTPage * pChildPage, vector<ObjectInfo> & tmpKeys, vector<BTPage *> & tmpSubPages);
            //CREO QUE TENGO QUE BORRARLOS AQUI
        // ForEach variadic con Perfect Forwarding
        // ForEach variadic
        template <typename Func, typename... Args>
        void forEach(size_t level, Func func, Args&&... args) {
            for (size_t i = 0; i < m_KeyCount; ++i) {
                if (m_SubPages[i]) m_SubPages[i]->forEach(level + 1, func, std::forward<Args>(args)...);
                func(m_Keys[i], level, std::forward<Args>(args)...);
            }
            if (m_SubPages[m_KeyCount]) m_SubPages[m_KeyCount]->forEach(level + 1, func, std::forward<Args>(args)...);
        }

        // FirstThat variadic
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

            // forEachPage variadic con Perfect Forwarding
            template <typename Func, typename... Args>
            void forEachPage(Level level, Func func, Args&&... args) {
                func(m_keyCount, level, std::forward<Args>(args)...);
                for (Size i = 0; i <= m_keyCount; ++i)
                    if (m_subPages[i]) m_subPages[i]->forEachPage(level + 1, func, std::forward<Args>(args)...);
            }
};
