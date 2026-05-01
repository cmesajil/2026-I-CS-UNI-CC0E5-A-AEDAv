#ifndef __CIRCULARLINKEDLIST_H__
#define __CIRCULARLINKEDLIST_H__

#include "linkedlist.h"




//usa el mismo nodo LL

template <typename T>
struct AscendingCLLTrait : BaseTrait<LLNode<T>, less<T>> {
};


template <typename T>
struct DescendingCLLTrait : BaseTrait<LLNode<T>, greater<T>> {
};

//iterator
template <typename Container>
class CLLForwardIterator: public general_iterator<Container, CLLForwardIterator<Container>>{
public:
    using MySelf = CLLForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Node   = typename Container::Node;
private:
    Node *m_start;
    bool  m_started;
public:
    CLLForwardIterator(Container *pContainer, Node *pNode): Parent(pContainer, pNode), m_start(pNode), m_started(false) {}
    CLLForwardIterator(Container *pContainer, Node *pNode, bool sentinel): Parent(pContainer, pNode), m_start(nullptr), m_started(sentinel) {}
    //operador++ forward
    MySelf& operator++(){
        if (this->m_pNode){
            m_started      = true;
            this->m_pNode  = this->m_pNode->getNext();
            if (this->m_pNode == m_start)
                this->m_pNode = nullptr;
        }
        return *this;
    }
};



template <typename Trait>
class CircularLinkedList : public LinkedList<Trait>{
public:
    using value_type       = typename Trait::value_type;
    using Node             = typename Trait::Node;
    using Comp             = typename Trait::Comp;
    using MySelf           = CircularLinkedList<Trait>;
    using forward_iterator = CLLForwardIterator<MySelf>;
    friend forward_iterator;
    using const_iterator = CLLForwardIterator<const LinkedList<Trait>>;
    friend const_iterator;

private:
    mutable shared_mutex m_mtx;


public:

    //iteradores
    forward_iterator begin() { return forward_iterator(this, this->m_pRoot); }
    forward_iterator end()   { return forward_iterator(this, nullptr, true); }
    const_iterator begin() const { return const_iterator(this, this->m_pRoot); }
    const_iterator end()   const { return const_iterator(this, nullptr); }

    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&...args){
        unique_lock<shared_mutex> lock(m_mtx);
        if (this->m_size == 0) return;
        for (auto &item : *this)
            func(item, std::forward<Args>(args)...);
    }

    //DONE : insert reutilizando LinkedList<Trait>::insert
    void insert(const value_type &value, Ref ref) override{
        shared_lock<shared_mutex> lock(m_mtx);
        // Caso lista vacía
           if (!this->m_pRoot) {
               LinkedList<Trait>::insert(value, ref);
               this->m_tail->setNext(this->m_pRoot);
               return;
           }

           // 1. romper circularidad
           this->m_tail->setNext(nullptr);

           // 2. usar insert base
           LinkedList<Trait>::insert(value, ref);

           // 3. restaurar circularidad
           this->m_tail->setNext(this->m_pRoot);

    }

    //constructores
    CircularLinkedList() : LinkedList<Trait>() {
    }


    //copy constructor
    CircularLinkedList(const CircularLinkedList &other) : LinkedList<Trait>() {
        // LinkedList<Trait>() ya se encargó de inicializar m_pRoot a nullptr, etc.
        // Ahora hacemos la lógica de copiado:
        shared_lock<shared_mutex> lock(other.m_mtx);
        if (!other.m_pRoot) return;

        Node *curr = other.m_pRoot;
        do {
            // Usamos push_back para ir creando la nueva lista
            push_back(curr->getData(), curr->getRef());
            curr = curr->getNext();
        } while (curr != other.m_pRoot);
    }

    //move constructor
    CircularLinkedList(CircularLinkedList &&other) : LinkedList<Trait>() {
        // Bloqueamos el mutex del otro objeto para moverlo de forma segura
        unique_lock<shared_mutex> lock(other.m_mtx);

        // Movemos los punteros desde la base del 'other' a la nuestra
        this->m_pRoot = std::exchange(other.m_pRoot, nullptr);
        this->m_tail  = std::exchange(other.m_tail,  nullptr);
        this->m_size  = std::exchange(other.m_size,  0);
    }

    //copy assignment
    CircularLinkedList &operator=(const CircularLinkedList &other){
        if (this != &other){
            clear();
            shared_lock<shared_mutex> lock(other.m_mtx);
            if (!other.m_pRoot) return *this;
            Node *curr = other.m_pRoot;
            do {
                push_back(curr->getData(), curr->getRef());
                curr = curr->getNext();
            } while (curr != other.m_pRoot);
        }
        return *this;
    }

    //move assignment
    CircularLinkedList &operator=(CircularLinkedList &&other){
        if (this != &other){
            clear();
            unique_lock<shared_mutex> lock(other.m_mtx);
            this->m_pRoot = std::exchange(other.m_pRoot, nullptr);
            this->m_tail  = std::exchange(other.m_tail,  nullptr);
            this->m_size  = std::exchange(other.m_size,  0);
        }
        return *this;
    }

    void clear(){
        unique_lock<shared_mutex> lock(m_mtx);
        if (!this->m_pRoot) return;


        this->m_tail->setNext(nullptr);


        Node *curr = this->m_pRoot;
        while (curr){
            Node *next = curr->getNext();
            delete curr;
            curr = next;
        }

        this->m_pRoot = nullptr;
        this->m_tail  = nullptr;
        this->m_size  = 0;
    }

    //destructor seguro
    virtual ~CircularLinkedList() {
        clear();
    }

    // push front
    void push_front(value_type value, Ref ref) override{
        unique_lock<shared_mutex> lock(m_mtx);
        Node *newNode = new Node(value, ref);
        if (!this->m_pRoot){
            newNode->setNext(newNode); // circulo de uno
            this->m_pRoot = newNode;
            this->m_tail  = newNode;
        }
        else{
            newNode->setNext(this->m_pRoot);
            this->m_tail->setNext(newNode); // cierre circular
            this->m_pRoot = newNode;
        }
        this->m_size++;
    }

    // reutiliza push_back
    void push_back(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node* newNode = new Node(value, ref);

        if (this->m_size == 0) {
            newNode->setNext(newNode); // Apunta a sí mismo
            this->m_pRoot = newNode;
            this->m_tail = newNode;
        } else {
            newNode->setNext(this->m_pRoot); // IMPORTANTE: Cierra el círculo
            this->m_tail->setNext(newNode);
            this->m_tail = newNode;
        }
        this->m_size++;
    }



    //pop front
    std::tuple<value_type, Ref> pop_front() override{
        unique_lock<shared_mutex> lock(m_mtx);
        if (!this->m_pRoot) throw runtime_error("La lista esta vacia");
        Node *temp   = this->m_pRoot;
        auto  result = std::make_tuple(temp->getData(), temp->getRef());
        if (this->m_size == 1){
            this->m_pRoot = nullptr;
            this->m_tail  = nullptr;
        }
        else{
            this->m_pRoot = temp->getNext();
            this->m_tail->setNext(this->m_pRoot); // reparar cierre circular
        }
        delete temp;
        this->m_size--;
        return result;
    }

    // pop back devuelve tupla (value, ref)
    std::tuple<value_type, Ref> pop_back() override{
        unique_lock<shared_mutex> lock(m_mtx);
        if (!this->m_pRoot) throw runtime_error("La lista esta vacia");
        auto result = std::make_tuple(this->m_tail->getData(), this->m_tail->getRef());
        if (this->m_size == 1){
            delete this->m_tail;
            this->m_pRoot = nullptr;
            this->m_tail  = nullptr;
        }
        else{
            Node *act = this->m_pRoot;
            while (act->getNext() != this->m_tail)
                act = act->getNext();
            delete this->m_tail;
            this->m_tail = act;
            this->m_tail->setNext(this->m_pRoot);
        }
        this->m_size--;
        return result;
    }

    //operator[]
    value_type &operator[](size_t index) override{
        shared_lock<shared_mutex> lock(m_mtx);
        if (index >= this->m_size) throw out_of_range("Indice fuera de rango");
        Node *act = this->m_pRoot;
        for (size_t i = 0; i < index; ++i)
            act = act->getNext();
        return act->getDataRef();
    }



    friend std::ostream& operator<<(std::ostream& os, const CircularLinkedList& list) {
        std::shared_lock<std::shared_mutex> lock(list.m_mtx);

        os << "[";
        print_list(os, list);
        os << "]";
        return os;
    }

    friend istream &operator>>(istream &is, CircularLinkedList &list){
        std::unique_lock<std::shared_mutex> lock(list.m_mtx);
        return read_list(is, list);
    }

    size_t size() const override{
        shared_lock<shared_mutex> lock(m_mtx);
        return this->m_size;
    }
};

#endif
