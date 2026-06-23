#include <iostream>
#include <cctype>
#include "BTree.h"
#include <mutex>

void DemoBTree() {
    // Definimos BT usando tu Trait con el tipo T3 =char
    using MyTrait = Tree23TraitAscending<T3>;
    BTree<MyTrait> bt;

    // 1. Inserción (Usando T3 y Ref)
    const T2 keys = "D1XJ2xTg8zKL9AhijOPQcEowRSp0NbW567BUfCqrs4FdtYZakHIuvGV3eMylmn";
    for (size_t i = 0; i < keys.size(); ++i) {
        bt.Insert(T3(keys[i]), Ref(i * i));
    }

    // 2. ForEach Variadic usando size_t para el nivel de manera nativa
    size_t letterCount = 0;
    std::mutex mtx;
    bt.forEach([&mtx](BTree<MyTrait>::Entry& e, size_t level, size_t& count) {
        if (isalpha(e.GetData())) {
            std::lock_guard<std::mutex> lock(mtx);
            ++count;
        }
    }, letterCount);
    cout << "Letras en el arbol: " << letterCount << "\n";

    // 3. FirstThat Variadic buscando un objetivo tipo T3 sin casteo manual
    T3 target = 'M';
    auto* entry = bt.firstThat([](BTree<MyTrait>::Entry& e, size_t level, T3 t) {
        return e.GetData() == t;
    }, target);

    if (entry) {
        cout << "firstThat('M') -> Encontrado con Ref: " << entry->GetRef() << "\n";
    }

    // 4. Bucle for-range clásico usando tu iterador in-order
    string inorderKeys;
    for (auto& entry : bt) {
        inorderKeys += entry;
    }
    cout << "Claves en orden: " << inorderKeys << "\n";
}
