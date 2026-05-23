#ifndef __AVLTREE_HASHTABLE_H__
#define __AVLTREE_HASHTABLE_H__

#include "vector.h"
#include "AvlBinaryTree.h"

#include <functional>
#include <optional>
#include <shared_mutex>
#include <mutex>
#include <iostream>
#include <utility>
#include <stdexcept>
#include <vector>
#include "../types.h" // Ref = long;

// ===============================================================
// 1. TRAITS TOTALMENTE COMPATIBLES
// ===============================================================
template<typename Key, typename HeightType = int>
struct AVLHashBucketTrait : public BaseTrait<AVLNode<Key, HeightType>, std::less<Key>> {
    using height_type = HeightType;
};

template<typename Key, typename HeightType = int>
struct HashBucketsVectorTrait {
    using BucketAVL  = AVLTree<AVLHashBucketTrait<Key, HeightType>>;
    using BucketPtr  = BucketAVL*;
    using value_type = BucketPtr;
    using Node       = VectorNode<BucketPtr>;
    using Comp       = std::less<BucketPtr>;
};

// Struct para dar soporte a Structured Bindings (C++17)
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
    struct tuple_size<AVLHashKeyValuePair<Key, Value>> : std::integral_constant<std::size_t, 2> {};

    template<std::size_t I, typename Key, typename Value>
    struct tuple_element<I, AVLHashKeyValuePair<Key, Value>> {
        using type = typename std::conditional<I == 0, const Key&, const Value&>::type;
    };
}

// ===============================================================
// 2. CLASS AVLHASHTABLE
// ===============================================================
template<
    typename Key,
    typename Value,
    typename Hash = std::hash<Key>,
    typename HeightType = int
>
class AVLHashTable {
public:
    using BucketType   = AVLTree<AVLHashBucketTrait<Key, HeightType>>;
    using BucketPtr    = BucketType*;
    using TableType    = Vector<HashBucketsVectorTrait<Key, HeightType>>;
    using size_type    = std::size_t;
    using KeyValuePair = AVLHashKeyValuePair<Key, Value>;

private:
    size_type     m_bucketCount;
    TableType     m_table;
    Hash          m_hashFunction;

    size_type getBucketIndex(const Key& key) const {
        return m_hashFunction(key) % m_bucketCount;
    }

public:
    // Constructor por defecto
    explicit AVLHashTable(size_type bucketCount = 101)
        : m_bucketCount(bucketCount), m_table(bucketCount)
    {
        for (size_type i = 0; i < m_bucketCount; ++i) {
            m_table.push_back(new BucketType(), 0);
        }
    }

    // Destructor (Basado en iterador del Vector que da punteros directos)
    ~AVLHashTable() {
        for (auto bucket : m_table) {
            if (bucket) {
                delete bucket;
            }
        }
    }

    // Constructor de Copia
    // Constructor de Copia
        AVLHashTable(const AVLHashTable& other)
            : m_bucketCount(other.m_bucketCount),
              m_table(),
              m_hashFunction(other.m_hashFunction)
        {
            for (const auto oldBucket : other.m_table) {
                BucketPtr newBucket = new BucketType();

                if (oldBucket) {
                    // Paso 1: Extraer las llaves de manera segura y liberar el mutex del AVL
                    std::vector<Key> keysInBucket;
                    oldBucket->ForEach([&](const Key& keyItem) {
                        keysInBucket.push_back(keyItem);
                    });

                    // Paso 2: Buscar y clonar los datos uno por uno fuera de ForEach
                    for (const auto& keyItem : keysInBucket) {
                        auto* avlNode = oldBucket->search(keyItem);
                        if (avlNode) {
                            Value* heapVal = new Value(*reinterpret_cast<Value*>(avlNode->getRef()));
                            newBucket->insert(keyItem, reinterpret_cast<Ref>(heapVal));
                        }
                    }
                }
                m_table.push_back(newBucket, 0);
            }
        }

    // Operador de Asignación por Copia
        AVLHashTable& operator=(const AVLHashTable& other) {
            if (this != &other) {
                for (auto bucket : m_table) {
                    if (bucket) delete bucket;
                }
                m_table = TableType();

                m_bucketCount = other.m_bucketCount;
                m_hashFunction = other.m_hashFunction;

                for (const auto oldBucket : other.m_table) {
                    BucketPtr newBucket = new BucketType();

                    if (oldBucket) {
                        // Paso 1: Extraer llaves y liberar mutex del AVL
                        std::vector<Key> keysInBucket;
                        oldBucket->ForEach([&](const Key& keyItem) {
                            keysInBucket.push_back(keyItem);
                        });

                        // Paso 2: Buscar y clonar nodos fuera de ForEach
                        for (const auto& keyItem : keysInBucket) {
                            auto* avlNode = oldBucket->search(keyItem);
                            if (avlNode) {
                                Value* heapVal = new Value(*reinterpret_cast<Value*>(avlNode->getRef()));
                                newBucket->insert(keyItem, reinterpret_cast<Ref>(heapVal));
                            }
                        }
                    }
                    m_table.push_back(newBucket, 0);
                }
            }
            return *this;
        }

    // Constructor de Movimiento
    AVLHashTable(AVLHashTable&& other) noexcept
        : m_bucketCount(std::exchange(other.m_bucketCount, 0)),
          m_table(std::move(other.m_table)),
          m_hashFunction(std::move(other.m_hashFunction))
    {
    }

    // Operador de Asignación por Movimiento
    AVLHashTable& operator=(AVLHashTable&& other) noexcept {
        if (this != &other) {
            for (auto bucket : m_table) {
                if (bucket) delete bucket;
            }

            m_bucketCount = std::exchange(other.m_bucketCount, 0);
            m_table = std::move(other.m_table);
            m_hashFunction = std::move(other.m_hashFunction);
        }
        return *this;
    }

    // Acceso por corchetes (REPARADO: Usa .getData() sobre el Node de Vector)
    Value& operator[](const Key& key) {
        size_type index = getBucketIndex(key);
        // m_table[index] devuelve VectorNode&, llamamos a .getData() para obtener BucketPtr
        BucketType& avlBucket = *(m_table[index].getData());
        auto* node = avlBucket.search(key);

        if (node == nullptr) {
            Value* pNewValue = new Value();
            avlBucket.insert(key, reinterpret_cast<Ref>(pNewValue));
            node = avlBucket.search(key);
            if (node == nullptr) {
                delete pNewValue;
                throw std::runtime_error("Fallo critico de insercion en arbol AVL.");
            }
        }
        return *reinterpret_cast<Value*>(node->getRef());
    }

    // Inserción directa (REPARADO: Usa .getData())
    void insert(const Key& key, const Value& value) {
        size_type index = getBucketIndex(key);
        BucketType& avlBucket = *(m_table[index].getData());

        auto* node = avlBucket.search(key);
        if (node) {
            *reinterpret_cast<Value*>(node->getRef()) = value;
        } else {
            Value* heapVal = new Value(value);
            avlBucket.insert(key, reinterpret_cast<Ref>(heapVal));
        }
    }

    // Búsqueda
    std::optional<Value> find(const Key& key) const {
        size_type index = getBucketIndex(key);
        const BucketType* avlBucket = m_table[index].getData();
        if (avlBucket) {
            auto* node = avlBucket->search(key);
            if (node != nullptr) {
                return *reinterpret_cast<Value*>(node->getRef());
            }
        }
        return std::nullopt;
    }

    // Verificación de existencia
    bool contains(const Key& key) const {
        size_type index = getBucketIndex(key);
        const BucketType* avlBucket = m_table[index].getData();
        return avlBucket && avlBucket->search(key) != nullptr;
    }

    // ===============================================================
    // CONST_ITERATOR
    // ===============================================================
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
                        BucketPtr bucket = m_pTable->m_table[m_currentBucket].getData();

                        if (bucket) {
                            // 1. Recolectamos todas las llaves del balde de forma segura.
                            // El mutex del AVL se bloqueará y se liberará limpiamente al terminar ForEach.
                            std::vector<Key> keysInBucket;
                            bucket->ForEach([&](const Key& keyItem) {
                                keysInBucket.push_back(keyItem);
                            });

                            // 2. Ahora que ForEach terminó y liberó el mutex,
                            // procesamos cada search() uno por uno sin conflictos de concurrencia.
                            for (const auto& keyItem : keysInBucket) {
                                auto* node = bucket->search(keyItem);
                                if (node) {
                                    m_flatElements.push_back({
                                        keyItem,
                                        *reinterpret_cast<Value*>(node->getRef())
                                    });
                                }
                            }

                            // Si encontramos elementos en este balde, detenemos la búsqueda para iterarlos
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

    void printDiagnostics() const {
        for (size_type i = 0; i < m_bucketCount; ++i) {
            std::cout << "Bucket [" << i << "]: ";
            const BucketType* bucket = m_table[i].getData();
            if (bucket) {
                bucket->printInOrder();
            }
            std::cout << "\n";
        }
    }
};

// ===============================================================
// OPERADORES DE FLUJO GENERALES
// ===============================================================
template<typename K, typename V, typename H, typename HT>
std::ostream& operator<<(std::ostream& os, const AVLHashTable<K, V, H, HT>& hashTable) {
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
std::istream& operator>>(std::istream& is, AVLHashTable<K, V, H, HT>& hashTable) {
    K key; V value;
    while (is >> key >> value) {
        hashTable.insert(key, value);
    }
    return is;
}

void DemoHash();

#endif // __AVLTREE_HASHTABLE_H__
