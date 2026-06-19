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

// 1. Elemento del nodo
ttemplate <typename Trait>
class BTreeNodeItem
{
    public:
        using value_type = typename Trait::value_type;

    private:
        value_type       m_data;
        Ref              m_ref;
        size_t           UseCounter;

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

// 2. Especializaciones de rasgos (Aquí inyectamos el número de llaves y políticas)
template <typename T, size_t MaxKeys, bool Unique = true>
struct BTreeAscendingTrait : public BaseTrait<T, std::less<typename T::value_type>> {
    static constexpr size_t max_keys = MaxKeys;
    static constexpr bool is_unique  = Unique;
};

template <typename T, size_t MaxKeys, bool Unique = true>
struct BTreeDescendingTrait : public BaseTrait<T, std::greater<typename T::value_type>> {
    static constexpr size_t max_keys = MaxKeys;
    static constexpr bool is_unique  = Unique;
};

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
};

#endif


// Si no lo encuentra, deberia decirme:
// cual es la posicion donde deberia estar
template <typename Container, typename ObjType>
int binary_search(Container& container, int first, int last, ObjType &object)
{
       if( first >= last )
               return first;
       while( first < last )
       {
               int mid = (first+last)/2;
               if( object == (ObjType)container[mid ] )
                       return mid;
               if( object > (ObjType)container[mid ] )
                       first = mid+1;
               else
                       last  = mid;
       }
       if( object <= (ObjType)container[first] )
               return first;
       return last;
}

template <typename Container, typename ObjType>
void insert_at(Container& container, const ObjType &object, int pos)
{
       int size = container.size();
       for(int i = size-2 ; i >= pos ; i--)
               container[i+1] = container[i];
       container[pos] =  object;
}

template <typename Container>
void remove(Container& container, int pos)
{
       int size = container.size();
       for(int i = pos+1 ; i < size ; i++)
               container[i-1] = container[i];
}

template <typename T>
CBTreePage<T>:: CBTreePage(int maxKeys, bool unique)
                                       : m_MaxKeys(maxKeys), m_Unique(unique), m_KeyCount(0)
{
       Create();
       SetMaxKeysForChilds(m_MaxKeys);
}

template <typename T>
CBTreePage<T>::~CBTreePage()
{
       Reset();
}

template <typename T>
bt_ErrorCode CBTreePage<T>::Insert(const T& m_data, const Ref m_ref)
{
       int pos = binary_search(m_Keys, 0, m_KeyCount, m_data);
       bt_ErrorCode error = bt_ok;

       if( pos < m_KeyCount && (T)m_Keys[pos] == m_data && m_Unique)
               return bt_duplicate; // this key is duplicate

       if( !m_SubPages[pos] ) // this is a leave
       {
               ::insert_at(m_Keys, ObjectInfo(m_data, m_ref), pos);
               NumberOfKeys()++;
               if( Overflow() )
                       return bt_overflow;
               return bt_ok;
       }
       else
       {
               // recursive insertion
               error = m_SubPages[pos]->Insert(m_data, m_ref);
               if( error == bt_overflow )
               {
                       if( !Redistribute1(pos) )
                               SplitChild(pos);
                       if( Overflow() )          // Propagate overflow
                               return bt_overflow;
                       return bt_ok;
               }
       }

       if( Overflow() ) // node overflow
               return bt_overflow;
       return bt_ok;
}

template <typename T>
bool CBTreePage<T>::Redistribute1(int &pos)
{
       if( m_SubPages[pos]->Underflow() )
       {        // nkol = Number of keys on left brother, nkor = Number of keys on right brother
               int nkol = 0,
                   nkor = 0;
               // is this the first element or there are more elements on right brother
               if( pos > 0 )
                       nkol = m_SubPages[pos-1]->NumberOfKeys();
               if( pos < NumberOfKeys() )
                       nkor = m_SubPages[pos+1]->NumberOfKeys();

               if( nkol > nkor )
                       if( m_SubPages[pos-1]->NumberOfKeys() > m_SubPages[pos-1]->MinNumberOfKeys() )
                               RedistributeL2R(pos-1); // bring elements from left brother
                       else
                               if( pos == NumberOfKeys() )
                                       return (--pos, false);
                               else
                                       return false;
               else //nkol < nkor )
                       if( m_SubPages[pos+1]->NumberOfKeys() > m_SubPages[pos+1]->MinNumberOfKeys() )
                               RedistributeR2L(pos+1); // bring elements from right brother
                       else
                               if( pos == 0 )
                                       return (++pos, false);
                               else
                                       return false;
       }
       else // it is due to overflow
       {
               int fcol = GetFreeCellsOnLeft(pos),   // Free Cells On Left
                   fcor = GetFreeCellsOnRight(pos);  // Free Cells On Right

               if( !fcol && !fcor && m_SubPages[pos]->IsFull() )
                       return false;
               if( fcol > fcor ) // There is more space on left
                       RedistributeR2L(pos);
               else
                       RedistributeL2R(pos);
       }
       return true;
}

template <typename T>
bool CBTreePage<T>::Redistribute2(int pos)
{
       assert( pos > 0 && pos < NumberOfKeys()  );
       assert( m_SubPages[pos-1] != 0 && m_SubPages[pos] != 0 && m_SubPages[pos+1] != 0 );
       assert( m_SubPages[pos-1]->Underflow() ||
                       m_SubPages[ pos ]->Underflow() ||
                       m_SubPages[pos+1]->Underflow() );

       if( m_SubPages[pos-1]->Underflow() )
       {        // Rotate R2L
               RedistributeR2L(pos+1);
               RedistributeR2L(pos);
               if( m_SubPages[pos-1]->Underflow() )
                       return false;
       }
       else if( m_SubPages[pos+1]->Underflow() )
       {        // Rotate L2R
               RedistributeL2R(pos-1);
               RedistributeL2R(pos);
               if( m_SubPages[pos+1]->Underflow() )
                       return false;
       }
       else // The problem is exactly at pos !
       {
               // Rotate L2R
               RedistributeL2R(pos-1);
               RedistributeR2L(pos+1);
               if( m_SubPages[pos]->Underflow() )
                       return false;
       }
       return true;
}

template <typename T>
void CBTreePage<T>::RedistributeR2L(int pos)
{
       BTPage  *pSource = m_SubPages[ pos ],
               *pTarget = m_SubPages[pos-1];

       while(pSource->GetNumberOfKeys() > pSource->MinNumberOfKeys() &&
             pTarget->GetNumberOfKeys() < pSource->GetNumberOfKeys() )
       {
               // Move from this page to the down-left page \/
               ::insert_at(pTarget->m_Keys, m_Keys[pos-1], pTarget->NumberOfKeys()++);
               // Move the pointer leftest pointer to the rightest position
               ::insert_at(pTarget->m_SubPages, pSource->m_SubPages[0], pTarget->NumberOfKeys());

               // Move the leftest element to the root
               m_Keys[pos-1] = pSource->m_Keys[0];

               // Remove the leftest element from rigth page
               ::remove(pSource->m_Keys    , 0);
               ::remove(pSource->m_SubPages, 0);
               pSource->NumberOfKeys()--;
       }
}

template <typename T>
void CBTreePage<T>::RedistributeL2R(int pos)
{
       BTPage  *pSource = m_SubPages[pos],
               *pTarget = m_SubPages[pos+1];
       while(pSource->GetNumberOfKeys() > pSource->MinNumberOfKeys() &&
                 pTarget->GetNumberOfKeys() < pSource->GetNumberOfKeys() )
       {
               // Move from this page to the down-RIGHT page \/
               ::insert_at(pTarget->m_Keys, m_Keys[pos], 0);
               // Move the pointer rightest pointer to the leftest position
               ::insert_at(pTarget->m_SubPages, pSource->m_SubPages[pSource->NumberOfKeys()], 0);
               pTarget->NumberOfKeys()++;

               // Move the rightest element to the root
               m_Keys[pos] = pSource->m_Keys[pSource->NumberOfKeys()-1];

               // Remove the leftest element from rigth page
               pSource->NumberOfKeys()--;
       }
}

template <typename T>
void CBTreePage<T>::SplitChild(int pos)
{
       BTPage  *pChild1 = 0, *pChild2 = 0;
       if( pos > 0 )                                    // is left page full ?
               if( m_SubPages[pos-1]->IsFull() )
               {
                       pChild1 = m_SubPages[pos-1];
                       pChild2 = m_SubPages[pos--];
               }
       if( pos < GetNumberOfKeys() )   // is right page full ?
               if( m_SubPages[pos+1]->IsFull() )
               {
                       pChild1 = m_SubPages[pos];
                       pChild2 = m_SubPages[pos+1];
               }

       int nKeys = pChild1->GetNumberOfKeys() + pChild2->GetNumberOfKeys() + 1;

       vector<ObjectInfo> tmpKeys;
       vector<BTPage *>   tmpSubPages;

       MovePage(pChild1, tmpKeys, tmpSubPages);
       tmpKeys    .push_back(m_Keys[pos]);
       MovePage(pChild2, tmpKeys, tmpSubPages);

       BTPage *pChild3 = 0;
       ObjectInfo oi1, oi2;
       SplitPageInto3(tmpKeys, tmpSubPages, pChild1, pChild2, pChild3, oi1, oi2);

       m_Keys    [pos] = oi1;
       m_SubPages[pos] = pChild1;

       ::insert_at(m_Keys, oi2, pos+1);
       ::insert_at(m_SubPages, pChild2, pos+1);
       NumberOfKeys()++;

       m_SubPages[pos+2] = pChild3;
}

template <typename T>
void CBTreePage<T>::SplitPageInto3(vector<ObjectInfo>& tmpKeys,
                                                vector<BTPage *>  & tmpSubPages,
                                                BTPage* &     pChild1,
                                                BTPage* &     pChild2,
                                                BTPage* &     pChild3,
                                                ObjectInfo                & oi1,
                                                ObjectInfo                & oi2)
{
       assert(tmpKeys.size() >= 8);
       assert(tmpSubPages.size() >= 9);
       if( !pChild1 )
               pChild1 = new BTPage(m_MaxKeysForChilds, m_Unique);

       pChild1->clear();
       int nKeys = (tmpKeys.size()-2)/3;
       int i = 0;
       for( ; i < nKeys; i++ )
       {
               pChild1->m_Keys    [i] = tmpKeys    [i];
               pChild1->m_SubPages[i] = tmpSubPages[i];
               pChild1->NumberOfKeys()++;
       }
       pChild1->m_SubPages[i] = tmpSubPages[i];

       oi1 = tmpKeys[i++];

       if( !pChild2 )
               pChild2 = new BTPage(m_MaxKeysForChilds, m_Unique);
       pChild2->clear();
       nKeys += (tmpKeys.size()-2)/3 + 1;
       int j = 0;
       for(; i < nKeys; i++, j++ )
       {
               pChild2->m_Keys    [j] = tmpKeys    [i];
               pChild2->m_SubPages[j] = tmpSubPages[i];
               pChild2->NumberOfKeys()++;
       }
       pChild2->m_SubPages[j] = tmpSubPages[i];

       oi2 = tmpKeys[i++];

       if( !pChild3 )
               pChild3 = new BTPage(m_MaxKeysForChilds, m_Unique);
       pChild3->clear();
       nKeys = tmpKeys.size();
       for(j = 0; i < nKeys; i++, j++)
       {
               pChild3->m_Keys    [j] = tmpKeys    [i];
               pChild3->m_SubPages[j] = tmpSubPages[i];
               pChild3->NumberOfKeys()++;
       }
       pChild3->m_SubPages[j] = tmpSubPages[i];
}

template <typename T>
bool CBTreePage<T>::SplitRoot()
{
       BTPage  *pChild1 = 0, *pChild2 = 0, *pChild3 = 0;
       ObjectInfo oi1, oi2;
       SplitPageInto3( m_Keys,m_SubPages,pChild1, pChild2, pChild3, oi1, oi2);
       clear();

       m_Keys    [0] = oi1;
       m_SubPages[0] = pChild1;
       NumberOfKeys()++;

       m_Keys    [1] = oi2;
       m_SubPages[1] = pChild2;
       NumberOfKeys()++;

       m_SubPages[2] = pChild3;
       return true;
}

template <typename T>
bool CBTreePage<T>::Search(const T &m_data, long &m_ref)
{
       int pos = binary_search(m_Keys, 0, m_KeyCount, m_data);
       if( pos >= m_KeyCount ){
               if( m_SubPages[pos] )
                       return m_SubPages[pos]->Search(m_data, m_ref);
               else
                       return false;
       }
       if( m_data == m_Keys[pos].m_data )
       {
               m_ref = m_Keys[pos].m_ref;
               m_Keys[pos].UseCounter++;
               return true;
       }
       if( m_data < m_Keys[pos].m_data )
               if( m_SubPages[pos] )
                       return m_SubPages[pos]->Search(m_data, m_ref);
       return false;
}

template <typename T>
void CBTreePage<T>::ForEach(lpfnForEach2 lpfn, int level, void *pExtra1)
{
       for( int i = 0 ; i < m_KeyCount ; i++)
       {
               if( m_SubPages[i] )
                       m_SubPages[i]->ForEach(lpfn, level+1, pExtra1);
               lpfn(m_Keys[i], level, pExtra1);
       }
       if( m_SubPages[m_KeyCount] )
               m_SubPages[m_KeyCount]->ForEach(lpfn, level+1, pExtra1);
}

template <typename T>
void CBTreePage<T>::ForEach(lpfnForEach3 lpfn, int level, void *pExtra1, void *pExtra2)
{
       for( int i = 0 ; i < m_KeyCount ; i++)
       {
               if( m_SubPages[i] )
                       m_SubPages[i]->ForEach(lpfn, level+1, pExtra1, pExtra2);
               lpfn(m_Keys[i], level, pExtra1, pExtra2);
       }
       if( m_SubPages[m_KeyCount] )
               m_SubPages[m_KeyCount]->ForEach(lpfn, level+1, pExtra1, pExtra2);
}

template <typename T>
typename CBTreePage<T>::ObjectInfo *
CBTreePage<T>::FirstThat(lpfnFirstThat2 lpfn, int level, void *pExtra1)
{
       ObjectInfo *pTmp;
       for( int i = 0 ; i < m_KeyCount ; i++)
       {
               if( m_SubPages[i] ){
                        pTmp = m_SubPages[i]->FirstThat(lpfn, level+1, pExtra1);
                       if( pTmp )
                               return pTmp;
               }
               if( lpfn(m_Keys[i], level, pExtra1) )
                       return &m_Keys[i];
       }
       if( m_SubPages[m_KeyCount] ){
                pTmp = m_SubPages[m_KeyCount]->FirstThat(lpfn, level+1, pExtra1);
               if( pTmp )
                       return pTmp;
       }
       return 0;
}

template <typename T>
typename CBTreePage<T>::ObjectInfo *
CBTreePage<T>::FirstThat(lpfnFirstThat3 lpfn,int level, void *pExtra1, void *pExtra2)
{
       ObjectInfo *pTmp;
       for( int i = 0 ; i < m_KeyCount ; i++){
               if( m_SubPages[i] ){
                       pTmp = m_SubPages[i]->FirstThat(lpfn, level+1, pExtra1, pExtra2);
                       if( pTmp )
                           return pTmp;
               }
               if( lpfn(m_Keys[i], level, pExtra1, pExtra2) )
                       return &m_Keys[i];
       }
        if( m_SubPages[m_KeyCount] )
        {       pTmp = m_SubPages[m_KeyCount]->FirstThat(lpfn, level+1, pExtra1, pExtra2);
                if( pTmp )
                return pTmp;
        }
        return 0;
}

template <typename T>
bt_ErrorCode CBTreePage<T>::Remove(const T &m_data, const Ref m_ref)
{
       bt_ErrorCode error = bt_ok;
       int pos = binary_search(m_Keys, 0, m_KeyCount, m_data);
       if( pos < NumberOfKeys() && m_data == m_Keys[pos].m_data )
       {
               if( !m_SubPages[pos+1] )
               {
                       ::remove(m_Keys, pos);
                       NumberOfKeys()--;
                       if( Underflow() )
                               return bt_underflow;
                       return bt_ok;
               }

               {
                       ObjectInfo &rFirstFromRight = m_SubPages[pos+1]->GetFirstObjectInfo();
                       swap(m_Keys[pos], rFirstFromRight);
                       error = m_SubPages[++pos]->Remove(m_data, m_ref);
               }
       }
       else if( pos == NumberOfKeys() )
               error = m_SubPages[pos]->Remove(m_data, m_ref);
       else if( m_data <= m_Keys[pos].m_data ){
               if( m_SubPages[pos] )
                       error = m_SubPages[pos]->Remove(m_data, m_ref);
               else
                       return bt_nofound;
       }
       if( error == bt_underflow ){
               if( TreatUnderflow(pos) )
                       return bt_ok;
               if( IsRoot() && NumberOfKeys() == 2 )
                       return MergeRoot();
               return Merge(pos);
       }
       if( error == bt_nofound )
               return bt_nofound;
       return bt_ok;
}

template <typename T>
bt_ErrorCode CBTreePage<T>::Merge(int pos)
{
       assert( m_SubPages[pos-1]->NumberOfKeys() +
                m_SubPages[ pos ]->NumberOfKeys() +
                m_SubPages[pos+1]->NumberOfKeys() ==
                3*m_SubPages[ pos ]->MinNumberOfKeys() - 1);

       vector<ObjectInfo> tmpKeys;
       vector<BTPage *>   tmpSubPages;

       BTPage  *pChild1 = m_SubPages[pos-1],
               *pChild2 = m_SubPages[ pos ],
               *pChild3 = m_SubPages[pos+1];
       MovePage(pChild1, tmpKeys, tmpSubPages);
       tmpKeys    .push_back(m_Keys[pos-1]);
       MovePage(pChild2, tmpKeys, tmpSubPages);
       tmpKeys    .push_back(m_Keys[ pos ]);
       MovePage(pChild3, tmpKeys, tmpSubPages);
       pChild3->Destroy();

       int nKeys = pChild1->GetFreeCells();
       int i = 0;
       for( ; i < nKeys ; i++ )
       {
               pChild1->m_Keys    [i] = tmpKeys    [i];
               pChild1->m_SubPages[i] = tmpSubPages[i];
               pChild1->NumberOfKeys()++;
       }
       pChild1->m_SubPages[i] = tmpSubPages[i];

       m_Keys    [pos-1] = tmpKeys[i];
       m_SubPages[pos-1] = pChild1;

       ::remove(m_Keys    , pos);
       ::remove(m_SubPages, pos);
       NumberOfKeys()--;

       nKeys = pChild2->GetFreeCells();
       int j = ++i;
       for(i = 0 ; i < nKeys ; i++, j++ )
       {
               pChild2->m_Keys    [i] = tmpKeys    [j];
               pChild2->m_SubPages[i] = tmpSubPages[j];
               pChild2->NumberOfKeys()++;
       }
       pChild2->m_SubPages[i] = tmpSubPages[j];
       m_SubPages[ pos ]          = pChild2;

       if( Underflow() )
               return bt_underflow;
       return bt_ok;
}

template <typename T>
bt_ErrorCode CBTreePage<T>::MergeRoot()
{
       int pos = 1;
       assert( m_SubPages[pos-1]->NumberOfKeys() +
                       m_SubPages[ pos ]->NumberOfKeys() +
                       m_SubPages[pos+1]->NumberOfKeys() ==
                       3*m_SubPages[ pos ]->MinNumberOfKeys() - 1);

       BTPage  *pChild1 = m_SubPages[pos-1], *pChild2 = m_SubPages[ pos ], *pChild3 = m_SubPages[pos+1];
       int nKeys = pChild1->NumberOfKeys() + pChild2->NumberOfKeys() + pChild3->NumberOfKeys() + 2;

       vector<ObjectInfo> tmpKeys;
       vector<BTPage *>   tmpSubPages;

       MovePage(pChild1, tmpKeys, tmpSubPages);
       tmpKeys    .push_back(m_Keys[pos-1]);
       MovePage(pChild2, tmpKeys, tmpSubPages);
       tmpKeys    .push_back(m_Keys[ pos ]);
       MovePage(pChild3, tmpKeys, tmpSubPages);

       clear();
       int i = 0;
       for( ; i < nKeys ; i++ ){
               m_Keys    [i] = tmpKeys    [i];
               m_SubPages[i] = tmpSubPages[i];
               NumberOfKeys()++;
       }
       m_SubPages[i] = tmpSubPages[i];

       pChild1->Destroy();
       pChild2->Destroy();
       pChild3->Destroy();

       return bt_rootmerged;
}

template <typename T>
typename CBTreePage<T>::ObjectInfo &
CBTreePage<T>::GetFirstObjectInfo()
{
       if( m_SubPages[0] )
               return m_SubPages[0]->GetFirstObjectInfo();
       return m_Keys[0];
}

template <typename T>
void Print(tagObjectInfo<T> &info, int level, void *pExtra)
{
        ostream &os = *(ostream *)pExtra;
        for( int i = 0; i < level ; i++)
                os << "\t";
        os << info.m_data << "->" << info.m_ref << "\n";
}

template <typename T>
void CBTreePage<T>::Print(ostream & os)
{
       lpfnForEach2 lpfn = &::Print<T>;
       ForEach(lpfn, 0, &os);
}

template <typename T>
void CBTreePage<T>::Create()
{
       Reset();
       m_Keys.resize(m_MaxKeys+1);
       m_SubPages.resize(m_MaxKeys+2, NULL);
       m_KeyCount = 0;
       m_MinKeys  = 2 * m_MaxKeys/3;
}

template <typename T>
void CBTreePage<T>::Reset()
{
       for( int i = 0 ; i < m_KeyCount ; i++ )
               delete m_SubPages[i];
       clear();
}

template <typename T>
void CBTreePage<T>::clear()
{
       m_KeyCount = 0;
}

template <typename T>
CBTreePage<T> * CreateBTreeNode (int maxKeys, int unique)
{
       return new CBTreePage<T> (maxKeys, unique);
}

template <typename T>
void CBTreePage<T>::MovePage(BTPage *pChildPage, vector<ObjectInfo> &tmpKeys,vector<BTPage *> &tmpSubPages)
{
       int nKeys = pChildPage->GetNumberOfKeys();
       int i = 0;
       for( ; i < nKeys; i++ )
       {
               tmpKeys    .push_back(pChildPage->m_Keys[i]);
               tmpSubPages.push_back(pChildPage->m_SubPages[i]);
       }
       tmpSubPages.push_back(pChildPage->m_SubPages[i]);
       pChildPage->clear();
}

template <typename T>
int CBTreePage<T>::GetFreeCellsOnLeft(int pos)
{
       if( pos > 0 )
               return m_SubPages[pos-1]->GetFreeCells();
       return 0;
}

template <typename T>
int CBTreePage<T>::GetFreeCellsOnRight(int pos)
{
       if( pos < GetNumberOfKeys() )
               return m_SubPages[pos+1]->GetFreeCells();
       return 0;
}

#endif
