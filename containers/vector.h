    #ifndef __VECTOR_H__
    #define __VECTOR_H__

    #include <iostream>
    #include <cstddef> // size_t
    #include <string>
    #include <sstream>
    #include <shared_mutex> // shared_mutex
    #include "general_iterator.h"
    #include "util.h"
    #include <mutex>
    #include "../types.h"
    #include <functional>
    #include "traits.h"
    using namespace std;

    template <typename Container>
    class vector_forward_iterator : public general_iterator<Container, vector_forward_iterator<Container>> {
    public:
        using MySelf = vector_forward_iterator<Container>;
        using Parent = general_iterator<Container, MySelf>;
        using Parent::Parent;
        MySelf& operator++() { this->m_pNode++; return *this; }
    };

    template <typename Container>
    class vector_backward_iterator : public general_iterator<Container, vector_backward_iterator<Container>> {
    public:
        using MySelf = vector_backward_iterator<Container>;
        using Parent = general_iterator<Container, MySelf>;
        using Parent::Parent;
        MySelf& operator++() { this->m_pNode--; return *this; }
    };



    template <typename T>
    class VectorNode{
        T   m_data;
        Ref m_ref;
    public:
        using value_type = T;
        VectorNode() : m_data(T()), m_ref(Ref()) {}
        VectorNode(T data, Ref ref) : m_data(data), m_ref(ref) {}
        VectorNode(const VectorNode &other) : m_data(other.m_data), m_ref(other.m_ref) {}
        VectorNode(VectorNode &&other) : m_data(move(other.m_data)), m_ref(move(other.m_ref)) {}
        VectorNode& operator=(const VectorNode &other) {
            m_data = other.m_data;
            m_ref = other.m_ref;
            return *this;
        }
        VectorNode& operator=(VectorNode &&other) {
            if(this != &other){
                m_data = move(other.m_data);
                m_ref = move(other.m_ref);
                return *this;
            }
            return *this;

        }

        const T&    getData() const { return m_data; }
        T&   getDataRef() { return m_data; }
        void setData(T data) { m_data = data; }
        Ref  getRef() { return m_ref; }
        void setRef(Ref ref) { m_ref = ref; }

    };

    template <typename T>
    ostream& operator<<(ostream& os, VectorNode<T>& node){
        return os << "(" << node.getData() << ", " << node.getRef() << ")";
    }

    // Traits de Ordenamiento
    template <typename T>
    struct AscendingVectorTrait : BaseTrait<VectorNode<T>,  less<T>>{
    };

    template <typename T>
    struct DescendingVectorTrait : BaseTrait<VectorNode<T>, greater<T>>{
    };

    template <typename Trait>
    class Vector{
    public:
    public:
        using value_type = typename Trait::value_type;
        using Node       = typename Trait::Node;
        using Comp       = typename Trait::Comp;
        using MySelf     = Vector<Trait>;
        using  forward_iterator   = vector_forward_iterator < MySelf > ;
        friend forward_iterator;

        using const_iterator = vector_forward_iterator<const Vector<Trait>>;
        friend const_iterator;

        using  backward_iterator  = vector_backward_iterator< MySelf > ;
        friend backward_iterator;


    private:
        size_t  m_capacity;
        size_t  m_size;
        Node   *m_data;
        mutable shared_mutex m_mtx;
        void    resize();

        void pop_back_unsafe() {
            // Sin candados. Diseñada solo para estructuras hijas como el Heap que ya están protegidas.
            if (m_size > 0) {
                m_size--;
            }
        }
    public:
        Vector(size_t capacity = 10);
        virtual ~Vector();
        virtual void clear();
        virtual void push_back(value_type value, Ref ref);
        virtual size_t size() const;
        virtual string toString() const;
        virtual void pop_back();
        virtual bool empty() const {
            shared_lock<shared_mutex> lock(m_mtx);
            return m_size == 0;
        }


        // Copy Constructor usando iteradores y push_back
        Vector(const Vector &other) : m_capacity(10), m_size(0) {

            m_data = new Node[m_capacity];
            // Bloqueamos el otro vector para lectura segura
            std::shared_lock<std::shared_mutex> lock(other.m_mtx);

            // Opcional: Si tienes un método reserve(), puedes usarlo aquí
            // para evitar múltiples reasignaciones de memoria:
            // this->reserve(other.m_capacity);

            // Recorremos usando tu iterador (compila gracias a begin() y end() const)
            for (const auto& node : other) {
                // push_back recibe (value, ref) según tu firma previa:
                // virtual void push_back(value_type value, Ref ref);
                this->push_back(node.getData(), node.getRef());
            }
        }

        // Move Constructor
        Vector(Vector &&other) noexcept : m_capacity(0), m_size(0), m_data(nullptr) {
            // Bloqueamos el objeto que va a "morir" para robar sus datos de forma segura
            unique_lock<shared_mutex> lockOther(other.m_mtx);

            // Robamos los punteros y valores primitivos
            m_data     = std::exchange(other.m_data, nullptr);
            m_size     = std::exchange(other.m_size, 0);
            m_capacity = std::exchange(other.m_capacity, 0);

            // NOTA: El mutex 'm_mtx' de este nuevo Vector se inicializa solo.
            // El candado 'lockOther' se libera automáticamente al salir de este scope.
        }

        //copy assigment
        Vector& operator=(const Vector &other) {
            // 1. Evitar autoasignación (ej: v1 = v1;)
            if (this == &other) {
                return *this;
            }

            // 2. Bloquear ambos vectores para evitar condiciones de carrera (Race Conditions)
            // Usamos std::lock para bloquear ambos mutexes simultáneamente sin causar Deadlocks
            std::unique_lock<shared_mutex> lockThis(m_mtx, std::defer_lock);
            std::shared_lock<shared_mutex> lockOther(other.m_mtx, std::defer_lock);
            std::lock(lockThis, lockOther);

            // 3. Liberar la memoria que ya teníamos asignada
            clear();

            // 4. Copiar los atributos primitivos
            m_capacity = other.m_capacity;
            m_size = other.m_size;

            // 5. Reservar nueva memoria y copiar elemento por elemento
            // Recorremos usando tu iterador (compila gracias a begin() y end() const)
            for (const auto& node : other) {
                // push_back recibe (value, ref) según tu firma previa:
                // virtual void push_back(value_type value, Ref ref);
                this->push_back(node.getData(), node.getRef());
            }
            return *this; // Retornamos la referencia al objeto actual

        }
        //move assigment
        Vector& operator=(Vector &&other) noexcept {
            // 1. Evitar autoasignación por movimiento (ej: v1 = std::move(v1);)
            if (this == &other) {
                return *this;
            }

            // 2. Bloquear ambos objetos con candado exclusivo (ambos van a cambiar)
            std::unique_lock<shared_mutex> lockThis(m_mtx, std::defer_lock);
            std::unique_lock<shared_mutex> lockOther(other.m_mtx, std::defer_lock);
            std::lock(lockThis, lockOther);

            // 3. Limpiar nuestra memoria actual para no dejar fugas
            clear();

            // 4. Intercambiar/Robar los recursos usando std::exchange
            m_data     = std::exchange(other.m_data, nullptr);
            m_size     = std::exchange(other.m_size, 0);
            m_capacity = std::exchange(other.m_capacity, 0);

            return *this;
        }

        // Dentro de la sección pública de class Vector:
        // referencias inválidas tras modificaciones concurrentes
        Node& operator[](size_t index) {
            std::shared_lock<std::shared_mutex> lock(m_mtx);
            // Nota: Para máxima seguridad concurrencial interna, operaciones complejas como el Swap
            // se controlan externamente o asumen que el Heap ya bloqueó el contexto si fuese necesario,
            // pero para acceso crudo directo a los elementos del arreglo:
            return m_data[index];
        }
        //referencias inválidas tras modificaciones concurrentes
        const Node& operator[](size_t index) const {
            return m_data[index];
        }

        forward_iterator begin() { return forward_iterator(this, m_data); }
        forward_iterator end()   { return forward_iterator(this, m_data + m_size); }

        const_iterator begin() const { return const_iterator(this, m_data); }
        const_iterator end()   const { return const_iterator(this, m_data + m_size); }

        backward_iterator rbegin() { return backward_iterator(this, m_data + m_size - 1); }
        backward_iterator rend()   { return backward_iterator(this, m_data - 1); }

        // Done: Agregar control concurrente
        template <typename Func, typename... Args>
        void ForEach(Func func, Args &&...  args){
            unique_lock<shared_mutex> lock(m_mtx);
            ::ForEach(begin(), end(), func, std::forward<Args>(args)... );
        }

        // Done: Agregar control concurrente
        template <typename Func, typename... Args>
        void ReverseForEach(Func func, Args &&...  args){
            unique_lock<shared_mutex> lock(m_mtx);
            if(m_size == 0)
                return;
            ::ForEach(rbegin(), rend(), func, std::forward<Args>(args)... );
        }



    };


    template <typename Trait>
    Vector<Trait>::Vector(size_t capacity){
        m_capacity = capacity;
        m_size = 0;
        m_data = new Node[capacity];
    }

    template <typename Trait>
    Vector<Trait>::~Vector(){
        unique_lock<shared_mutex> lock(m_mtx);
        clear();
    }

    template <typename Trait>
    void Vector<Trait>::clear()
    {

        delete[] m_data;

        m_data = nullptr;
        m_size = 0;
        m_capacity = 0;
    }



    template <typename Trait>
    void Vector<Trait>::resize(){
        m_capacity = (m_capacity < 10) ? m_capacity+10 : m_capacity * 2;
        Node * new_data = new Node[m_capacity];
        for(size_t i = 0; i < m_size; ++i)
            new_data[i] = std::move(m_data[i]);
        delete [] m_data;
        m_data = new_data;
    }

    template <typename Trait>
    void Vector<Trait>::push_back(value_type value, Ref ref){
        unique_lock<shared_mutex> lock(m_mtx);
        if(m_size == m_capacity){
            if (m_capacity == 0) {
                m_capacity = 10;
                m_data = new Node[m_capacity];
            }
             resize();

        } // Overflow

        m_data[m_size++] = Node(value, ref);
    }

    template <typename Trait>
    size_t Vector<Trait>::size() const{
        shared_lock<shared_mutex> lock(m_mtx);
        return m_size;
    }

    template <typename Trait>
    string Vector<Trait>::toString() const{
        shared_lock<shared_mutex> lock(m_mtx);
        ostringstream oss;
        oss << "[";
        for(size_t i = 0; i < m_size; ++i){
            if(i > 0)
                oss << ",";
            oss << m_data[i];
        }
        oss << "]";
        return oss.str();
    }

    template <typename Trait>
    ostream& operator<<(ostream& os, const Vector<Trait>& v){
        return os << v.toString();
    }

    // TODO: Implementar
    template <typename Trait>
    istream& operator>>(istream& is, Vector<Trait>& v){
        return is;
    }
    // 2. Implementación al final del archivo
    template <typename Trait>
    void Vector<Trait>::pop_back(){
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_size > 0) {
            m_size--;
            // Como manejamos primitivos o tipos estándar mediante la asignación del nodo,
            // decrementar m_size es suficiente. El espacio queda disponible para futuros push_back.
        }
    }
    // template <typename T>
    // template <typename Func, typename... Args>
    // void Vector<T>::ForEach(Func func, Args &&...  args){
    //     ::ForEach(begin(), end(), func, std::forward<Args>(args)... );
    // }

    void DemoVector();
    void DemoConcurrentVector();

    #endif // __VECTOR_H__
