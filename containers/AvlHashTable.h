#ifndef __AVLTREE_HASHTABLE_H__
#define __AVLTREE_HASHTABLE_H__

#include <functional>
#include <iostream>
#include <utility>
#include <vector>
#include <cstddef>
#include <stdexcept>

#include "vector.h"
#include "AvlBinaryTree.h"
#include "../types.h"

// ===============================================================
// 1. TRAITS Y ESTRUCTURAS DE SOPORTE PARA ESTRUCTURING BINDING
// ===============================================================
template<typename Key, typename HeightType = size_t>
struct AVLHashBucketTrait : public BaseTrait<AVLNode<Key, HeightType>, std::less<Key>> {
    using height_type = HeightType;
    using node_type   = AVLNode<Key, HeightType>;
};

template<typename Key, typename Value, typename HeightType = size_t>
struct HashBucketsVectorTrait {
    using BucketAVL   = AVLTree<AVLHashBucketTrait<Key, HeightType>>;
    using BucketPtr   = BucketAVL*;
    using value_type  = BucketPtr;
    using Node        = VectorNode<BucketPtr>;
    using Comp        = std::less<BucketPtr>;
    using key_type    = Key;
    using mapped_type = Value;
};

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
    typename VectorTrait,
    typename Hash = std::hash<typename VectorTrait::key_type>
>
class AVLHashTable {
public:
    using Key          = typename VectorTrait::key_type;
    using Value        = typename VectorTrait::mapped_type;
    using BucketType   = typename VectorTrait::BucketAVL;
    using BucketPtr    = typename VectorTrait::BucketPtr;
    using TableType    = Vector<VectorTrait>;
    using size_type    = std::size_t;
    using KeyValuePair = AVLHashKeyValuePair<Key, Value>;

private:
    size_type     m_bucketCount;
    TableType     m_table;
    Hash          m_hashFunction;

    size_type getBucketIndex(const Key& key) const {
        return m_hashFunction(key) % m_bucketCount;
    }

    void setBucketAt(size_type index, BucketPtr newBucket) {
        m_table[index].setData(newBucket);
    }

public:
    explicit AVLHashTable(size_type bucketCount = 101)
        : m_bucketCount(bucketCount), m_table(bucketCount)
    {
        for (size_type i = 0; i < m_bucketCount; ++i) {
            setBucketAt(i, new BucketType());
        }
    }

    ~AVLHashTable() {
        for (size_type i = 0; i < m_bucketCount; ++i) {
            BucketPtr bucket = m_table[i].getData();
            if (bucket) {
                std::vector<Value*> valuesToDelete;
                bucket->ForEach([&](const Key& keyItem) {
                    auto* node = bucket->search(keyItem);
                    if (node && node->getRef()) {
                        valuesToDelete.push_back(reinterpret_cast<Value*>(node->getRef()));
                    }
                });
                delete bucket;
                for (Value* val : valuesToDelete) delete val;
            }
        }
    }

    // 1. CONSTRUCTOR COPIA
    AVLHashTable(const AVLHashTable& other)
        : m_bucketCount(other.m_bucketCount),
          m_table(other.m_bucketCount),
          m_hashFunction(other.m_hashFunction)
    {
        for (size_type i = 0; i < m_bucketCount; ++i) {
            setBucketAt(i, new BucketType());
        }

        for (size_type i = 0; i < m_bucketCount; ++i) {
            const auto oldBucket = other.m_table[i].getData();
            if (oldBucket) {
                std::vector<std::pair<Key, Value>> extractedData;
                oldBucket->ForEach([&](const Key& keyItem) {
                    auto* avlNode = oldBucket->search(keyItem);
                    if (avlNode && avlNode->getRef()) {
                        extractedData.push_back({keyItem, *reinterpret_cast<Value*>(avlNode->getRef())});
                    }
                });

                BucketPtr newBucket = m_table[i].getData();
                if (newBucket) {
                    for (const auto& [keyItem, rawValue] : extractedData) {
                        Value* heapVal = new Value(rawValue);
                        newBucket->insert(keyItem, reinterpret_cast<Ref>(heapVal));
                    }
                }
            }
        }
    }

    // 2. MOVE CONSTRUCTOR
    AVLHashTable(AVLHashTable&& other) noexcept
        : m_bucketCount(std::exchange(other.m_bucketCount, 0)),
          m_table(std::move(other.m_table)),
          m_hashFunction(std::move(other.m_hashFunction))
    {
        for (size_type i = 0; i < other.m_bucketCount; ++i) {
            other.setBucketAt(i, nullptr);
        }
    }

    // 3. ASIGNACIÓN ACCESO m[5] = 3
    Value& operator[](const Key& key) {
        size_type index = getBucketIndex(key);
        BucketType& avlBucket = *(m_table[index].getData());
        auto* node = avlBucket.search(key);

        if (node == nullptr) {
            Value* pNewValue = new Value();
            avlBucket.insert(key, reinterpret_cast<Ref>(pNewValue));
            node = avlBucket.search(key);
            if (node == nullptr) {
                delete pNewValue;
                throw std::runtime_error("Fallo critico de insercion AVL.");
            }
        }
        return *reinterpret_cast<Value*>(node->getRef());
    }

    // 4. CONST_ITERATOR PARA SOPORTAR: for (const auto& [key, value] : m)
    class ConstIterator {
    public:
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
                    std::vector<Key> keysInBucket;
                    bucket->ForEach([&](const Key& keyItem) { keysInBucket.push_back(keyItem); });

                    for (const auto& keyItem : keysInBucket) {
                        auto* node = bucket->search(keyItem);
                        if (node) {
                            m_flatElements.push_back({keyItem, *reinterpret_cast<Value*>(node->getRef())});
                        }
                    }
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
            if (m_currentBucket >= m_pTable->m_bucketCount && other.m_currentBucket >= other.m_pTable->m_bucketCount) {
                return false;
            }
            if (m_currentBucket != other.m_currentBucket) return true;
            return m_elementIndex != other.m_elementIndex;
        }
    };

    ConstIterator begin() const { return ConstIterator(this, 0); }
    ConstIterator end() const   { return ConstIterator(this, m_bucketCount); }
};

// 5. OPERATOR <<
template<typename VectorTrait, typename Hash>
std::ostream& operator<<(std::ostream& os, const AVLHashTable<VectorTrait, Hash>& hashTable) {
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

// 6. OPERATOR >>
template<typename VectorTrait, typename Hash>
std::istream& operator>>(std::istream& is, AVLHashTable<VectorTrait, Hash>& hashTable) {
    typename VectorTrait::key_type key;
    typename VectorTrait::mapped_type value;
    if (is >> key >> value) {
        hashTable[key] = value; // Usa directamente el operator[] que ya tenemos
    }
    return is;
}

void DemoHash();
#endif // __AVLTREE_HASHTABLE_H__
