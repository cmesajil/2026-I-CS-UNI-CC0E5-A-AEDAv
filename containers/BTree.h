// btree.h

#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include "BTreePage.h"

#define DEFAULT_BTREE_ORDER 3

template <typename Trait>
class BTree
// this is the full version of the BTree
{

    using Page = BTreePage<Trait>; // Tu clase página donde está el código que enviaste
    Page* m_root = nullptr;
    typedef CBTreePage <keyType, ObjIDType> BTNode;// useful shorthand
    /*struct ObjectInfo
    {
        keyType first;
        long    second;
        ObjectInfo *&operator->() { return this; }
    };*/

public:
       //typedef ObjectInfo iterator;
       typedef typename BTNode::lpfnForEach2    lpfnForEach2;
       typedef typename BTNode::lpfnForEach3    lpfnForEach3;
       typedef typename BTNode::lpfnFirstThat2  lpfnFirstThat2;
       typedef typename BTNode::lpfnFirstThat3  lpfnFirstThat3;
       typedef typename BTNode::ObjectInfo      ObjectInfo;

public:
       BTree(int order = DEFAULT_BTREE_ORDER, bool unique = true);
       ~BTree();
       //int           Open (char * name, int mode);
       //int           Create (char * name, int mode);
       //int           Close ();
       bool            Insert (const keyType key, const int ObjID);
       bool            Remove (const keyType key, const int ObjID);
       ObjIDType       Search (const keyType key);
       long            size()  { return m_NumKeys; }
       long            height() { return m_Height;      }
       long            GetOrder() { return m_Order;     }

       void            Print (ostream &os);
       void            ForEach( lpfnForEach2 lpfn, void *pExtra1 );
       void            ForEach( lpfnForEach3 lpfn, void *pExtra1, void *pExtra2);
       ObjectInfo*     FirstThat( lpfnFirstThat2 lpfn, void *pExtra1 );
       ObjectInfo*     FirstThat( lpfnFirstThat3 lpfn, void *pExtra1, void *pExtra2);
       //typedef               ObjectInfo iterator;

protected:
       BTNode          m_Root;
       int             m_Height;  // height of tree
       int             m_Order;   // order of tree
       long            m_NumKeys; // number of keys
       bool            m_Unique;  // Accept the elements only once ?



public:
    // Envoltura para forEach
    template <typename Func, typename... Args>
    void forEach(Func&& func, Args&&... args) {
        if (m_root) {
            m_root->forEach(0, std::forward<Func>(func), std::forward<Args>(args)...);
        }
    }

    // Envoltura para firstThat
    template <typename Func, typename... Args>
    Entry* firstThat(Func&& func, Args&&... args) {
        if (m_root) {
            return m_root->firstThat(0, std::forward<Func>(func), std::forward<Args>(args)...);
        }
        return nullptr;
    }

    // Iteradores para el for automático (for-range)
    using iterator = BTreeForwardIterator<BTree<Trait>>;
    iterator begin() { return iterator(this, m_root); }
    iterator end()   { return iterator(this); }

    // Envoltura para forEachPage
    template <typename Func, typename... Args>
    void forEachPage(Func&& func, Args&&... args) {
        if (m_root) {
            m_root->forEachPage(0, std::forward<Func>(func), std::forward<Args>(args)...);
        }
    }
};


#endif
