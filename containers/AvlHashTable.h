#ifndef __AVLTREE_HASHTABLE_H__
#define __AVLTREE_HASHTABLE_H__

#include "AvlBinaryTree.h"
#include "vector.h"

#include <functional>
#include <iterator>
#include <optional>
#include <shared_mutex>
#include <mutex>
#include <vector>
#include <iostream>
#include <utility>
#include <memory>
#include <stdexcept>

using Ref = long int;

///////////////////////////////////////////////////////////////
// 1. TRAIT DEL AVL PARA LOS BALDES
///////////////////////////////////////////////////////////////
template<typename Key, typename HeightType = size_t>
struct AVLHashBucketTrait
    : public BaseTrait<
        AVLNode<Key, HeightType>,
        std::less<Key>
    >
{
    using height_type = HeightType;
};

///////////////////////////////////////////////////////////////
// 2. TRAIT DEL VECTOR
///////////////////////////////////////////////////////////////
template<typename Key, typename HeightType = size_t>
struct HashBucketsVectorTrait
{
    using BucketAVL = AVLTree<AVLHashBucketTrait<Key, HeightType>>;
    using BucketPtr = BucketAVL*;
    using value_type = BucketPtr;
    using Node = VectorNode<BucketPtr>;
    using Comp = std::less<BucketPtr>;
};

///////////////////////////////////////////////////////////////
// 2.5 STRUCTURED BINDINGS SUPPORT
///////////////////////////////////////////////////////////////
template <typename Key, typename Value>
struct AVLHashKeyValuePair {
    const Key& key;
    const Value& value;

    template <std::size_t I>
    const auto& get() const {
        if constexpr (I == 0) return key;
        else if constexpr (I == 1) return value;
    }
};

namespace std {
    template<typename Key, typename Value>
    struct tuple_size<AVLHashKeyValuePair<Key, Value>>
        : std::integral_constant<std::size_t, 2> {};

    template<std::size_t I, typename Key, typename Value>
    struct tuple_element<I, AVLHashKeyValuePair<Key, Value>> {
        using type = typename std::conditional<I == 0, const Key&, const Value&>::type;
    };
}

///////////////////////////////////////////////////////////////
// 3. HASH TABLE
///////////////////////////////////////////////////////////////
template<
    typename Key,
    typename Value,
    typename Hash = std::hash<Key>,
    typename HeightType = int
>
class AVLHashTable
{
public:
    using BucketType = AVLTree<AVLHashBucketTrait<Key, HeightType>>;
    using BucketPtr  = BucketType*;
    using TableType  = Vector<HashBucketsVectorTrait<Key, HeightType>>;
    using size_type  = std::size_t;

    using KeyValuePair = AVLHashKeyValuePair<Key, Value>;

private:
    size_type m_bucketCount;
    TableType m_table; // Su mutex interno provoca que no se pueda copiar/mover automáticamente
    Hash m_hashFunction;
    std::unique_ptr<std::shared_mutex[]> m_locks;

    size_type getBucketIndex(const Key& key) const {
        return m_hashFunction(key) % m_bucketCount;
    }

    void allocateLocks(size_type count) {
        if (count > 0) {
            m_locks = std::make_unique<std::shared_mutex[]>(count);
        } else {
            m_locks.reset();
        }
    }

public:
    explicit AVLHashTable(size_type bucketCount = 101)
        : m_bucketCount(bucketCount), m_table(bucketCount) {
        allocateLocks(m_bucketCount);
        for (size_type i = 0; i < m_bucketCount; ++i) {
            m_table.push_back(new BucketType(), 0);
        }
    }

    ~AVLHashTable() {
        for (size_type i = 0; i < m_table.size(); ++i) {
            if (m_table[i].getData()) {
                delete m_table[i].getData();
            }
        }
    }

    // Constructor de copia manual - REPARADO (Anti-Deadlock)
        AVLHashTable(const AVLHashTable& other)
            : m_bucketCount(other.m_bucketCount), m_table(other.m_bucketCount), m_hashFunction(other.m_hashFunction) {
            allocateLocks(m_bucketCount);

            for (size_type i = 0; i < m_bucketCount; ++i) {
                BucketPtr oldBucket = nullptr;

                // 1. Bloqueamos solo el instante necesario para capturar el puntero de forma segura
                {
                    std::shared_lock<std::shared_mutex> readLock(other.m_locks[i]);
                    oldBucket = other.m_table[i].getData();
                } // El lock se libera AUTOMÁTICAMENTE aquí al salir de las llaves {}

                BucketPtr newBucket = new BucketType();

                // 2. Ahora, completamente libres de bloqueos externos, extraemos los datos
                if (oldBucket) {
                    std::vector<Key> keysInBucket;
                    oldBucket->ForEach([&keysInBucket](const Key& keyItem) {
                        keysInBucket.push_back(keyItem);
                    });

                    for (const auto& keyItem : keysInBucket) {
                        auto* node = oldBucket->search(keyItem);
                        if (node) {
                            // Copiar el valor dinámicamente en el Heap
                            Value* heapVal = new Value(*reinterpret_cast<Value*>(node->getRef()));
                            newBucket->insert(keyItem, reinterpret_cast<Ref>(heapVal));
                        }
                    }
                }
                m_table.push_back(newBucket, 0);
            }
        }

        // Asignación por copia manual - REPARADO (Anti-Deadlock)
        AVLHashTable& operator=(const AVLHashTable& other) {
            if (this != &other) {
                // Limpiar datos viejos de la tabla actual antes de recibir la nueva información
                for (size_type i = 0; i < m_table.size(); ++i) {
                    if (m_table[i].getData()) delete m_table[i].getData();
                }

                m_bucketCount = other.m_bucketCount;
                m_hashFunction = other.m_hashFunction;
                allocateLocks(m_bucketCount);

                m_table = TableType(m_bucketCount);

                for (size_type i = 0; i < m_bucketCount; ++i) {
                    BucketPtr oldBucket = nullptr;

                    // Extraemos el puntero bajo protección estricta de alcance
                    {
                        std::shared_lock<std::shared_mutex> readLock(other.m_locks[i]);
                        oldBucket = other.m_table[i].getData();
                    }

                    BucketPtr newBucket = new BucketType();

                    if (oldBucket) {
                        std::vector<Key> keysInBucket;
                        oldBucket->ForEach([&keysInBucket](const Key& keyItem) {
                            keysInBucket.push_back(keyItem);
                        });

                        for (const auto& keyItem : keysInBucket) {
                            auto* node = oldBucket->search(keyItem);
                            if (node) {
                                Value* heapVal = new Value(*reinterpret_cast<Value*>(node->getRef()));
                                newBucket->insert(keyItem, reinterpret_cast<Ref>(heapVal));
                            }
                        }
                    }
                    m_table.push_back(newBucket, 0);
                }
            }
            return *this;
        }

    // CONSTRUCTOR DE MOVIMIENTO MANUAL (¡Aquí rompemos el cuello de botella!)
    AVLHashTable(AVLHashTable&& other) noexcept
        : m_bucketCount(other.m_bucketCount),
          m_table(other.m_bucketCount), // Se inicializa vacío con la capacidad requerida
          m_hashFunction(std::move(other.m_hashFunction)),
          m_locks(std::move(other.m_locks))
    {
        // En vez de std::move(m_table) que invoca la función borrada de Vector,
        // robamos los punteros de los AVL directamente balde por balde de forma segura.
        for (size_type i = 0; i < other.m_table.size(); ++i) {
            m_table.push_back(other.m_table[i].getData(), 0);
            other.m_table[i].setData(nullptr); // Desvinculamos el origen para evitar doble delete
        }
        other.m_bucketCount = 0;
    }

    // Asignación por movimiento manual
    AVLHashTable& operator=(AVLHashTable&& other) noexcept {
        if (this != &other) {
            for (size_type i = 0; i < m_table.size(); ++i) {
                if (m_table[i].getData()) delete m_table[i].getData();
            }

            m_bucketCount = other.m_bucketCount;
            m_hashFunction = std::move(other.m_hashFunction);
            m_locks = std::move(other.m_locks);

            m_table = TableType(m_bucketCount);
            for (size_type i = 0; i < other.m_table.size(); ++i) {
                m_table.push_back(other.m_table[i].getData(), 0);
                other.m_table[i].setData(nullptr);
            }
            other.m_bucketCount = 0;
        }
        return *this;
    }

    Value& operator[](const Key& key) {
        size_type index = getBucketIndex(key);
        std::unique_lock<std::shared_mutex> writeLock(m_locks[index]);

        BucketType& avlBucket = *m_table[index].getDataRef();
        auto* node = avlBucket.search(key);

        if (node == nullptr) {
            Value* pNewValue = new Value();
            avlBucket.insert(key, reinterpret_cast<Ref>(pNewValue));
            node = avlBucket.search(key);
            if (node == nullptr) {
                delete pNewValue;
                throw std::runtime_error("Fallo critico de insercion.");
            }
        }
        return *reinterpret_cast<Value*>(node->getRef());
    }

    ///////////////////////////////////////////////////////////
        // CONST_ITERATOR REPARADO (Anti-Deadlock)
        ///////////////////////////////////////////////////////////
        class ConstIterator {
        private:
            const AVLHashTable* m_pTable;
            size_type m_currentBucket;
            std::vector<std::pair<Key, Value>> m_flatElements;
            size_type m_elementIndex;

            void flattenCurrentBucket() {
                m_flatElements.clear();
                m_elementIndex = 0;

                while (m_currentBucket < m_pTable->m_bucketCount) {
                    BucketPtr bucket = nullptr;

                    // 1. Bloqueamos solo para obtener el puntero al árbol de forma segura
                    {
                        std::shared_lock<std::shared_mutex> readLock(m_pTable->m_locks[m_currentBucket]);
                        bucket = m_pTable->m_table[m_currentBucket].getData();
                    } // El bloqueo se libera AUTOMÁTICAMENTE aquí al salir del bloque {}

                    // 2. Ahora que el cerrojo está libre, iteramos y buscamos de forma segura
                    //    sin riesgo de provocar interbloqueos (deadlocks) recursivos.
                    if (bucket) {
                        // Recolectamos primero todas las llaves del balde
                        std::vector<Key> keysInBucket;
                        bucket->ForEach([&keysInBucket](const Key& keyItem) {
                            keysInBucket.push_back(keyItem);
                        });

                        // Buscamos los nodos correspondientes
                        for (const auto& keyItem : keysInBucket) {
                            auto* node = bucket->search(keyItem);
                            if (node) {
                                m_flatElements.push_back({
                                    keyItem,
                                    *reinterpret_cast<Value*>(node->getRef())
                                });
                            }
                        }

                        // Si encontramos elementos en este balde, paramos para que el bucle los procese
                        if (!m_flatElements.empty()) return;
                    }
                    m_currentBucket++;
                }
            }

        public:
            ConstIterator(const AVLHashTable* table, size_type bucket)
                : m_pTable(table), m_currentBucket(bucket), m_elementIndex(0) {
                flattenCurrentBucket();
            }

            KeyValuePair operator*() const {
                return KeyValuePair{ m_flatElements[m_elementIndex].first, m_flatElements[m_elementIndex].second };
            }

            ConstIterator& operator++() {
                m_elementIndex++;
                if (m_elementIndex >= m_flatElements.size()) {
                    m_currentBucket++;
                    flattenCurrentBucket();
                }
                return *this;
            }

            bool operator!=(const ConstIterator& other) const {
                if (m_currentBucket != other.m_currentBucket) return true;
                if (m_currentBucket >= m_pTable->m_bucketCount) return false;
                return m_elementIndex != other.m_elementIndex;
            }
        };

    ConstIterator begin() const { return ConstIterator(this, 0); }
    ConstIterator end() const   { return ConstIterator(this, m_bucketCount); }

    void insert(const Key& key, const Value& value) {
        size_type index = getBucketIndex(key);
        std::unique_lock<std::shared_mutex> writeLock(m_locks[index]);
        BucketType& avlBucket = *m_table[index].getDataRef();

        auto* node = avlBucket.search(key);
        if (node) {
            *reinterpret_cast<Value*>(node->getRef()) = value;
        } else {
            Value* heapVal = new Value(value);
            avlBucket.insert(key, reinterpret_cast<Ref>(heapVal));
        }
    }

    std::optional<Value> find(const Key& key) const {
        size_type index = getBucketIndex(key);
        std::shared_lock<std::shared_mutex> readLock(m_locks[index]);
        const BucketType& avlBucket = *m_table[index].getData();
        auto* node = avlBucket.search(key);
        if (node != nullptr) {
            return *reinterpret_cast<Value*>(node->getRef());
        }
        return std::nullopt;
    }

    bool contains(const Key& key) const {
        size_type index = getBucketIndex(key);
        std::shared_lock<std::shared_mutex> readLock(m_locks[index]);
        const BucketType& avlBucket = *m_table[index].getData();
        return avlBucket.search(key) != nullptr;
    }

    void printDiagnostics() const {
        for (size_type i = 0; i < m_bucketCount; ++i) {
            std::shared_lock<std::shared_mutex> readLock(m_locks[i]);
            std::cout << "Bucket [" << i << "]: ";
            const BucketType& avlBucket = *m_table[i].getData();
            avlBucket.printInOrder();
            std::cout << "\n";
        }
    }
};

///////////////////////////////////////////////////////////////
// OPERADORES DE FLUJO
///////////////////////////////////////////////////////////////
template<typename K, typename V, typename H, typename HT>
std::ostream& operator<<(std::ostream& os, const AVLHashTable<K, V, H, HT>& hashTable)
{
    os << "{";
    bool first = true;
    for (const auto& item : hashTable) {
        if (!first) os << ", ";
        os << item.key << ": " << item.value;
        first = false;
    }
    os << "}";
    return os;
}

template<typename K, typename V, typename H, typename HT>
std::istream& operator>>(std::istream& is, AVLHashTable<K, V, H, HT>& hashTable)
{
    K key;
    V value;
    while (is >> key >> value) {
        hashTable.insert(key, value);
    }
    return is;
}

void DemoHash();

#endif // __AVLTREE_HASHTABLE_H__
