#include <iostream>
#include <cctype>
#include "BTree.h"
//que obtenga los parametros
// =========================================================================
// FUNCIÓN DEMO ACTUALIZADA
// =========================================================================
void DemoBTree() {
    using MyTrait = Tree23TraitAscending<T3>;
    BTree<MyTrait> bt;

    const T2 keys =
        "D1XJ2xTg8zKL9AhijOPQcEowRSp0NbW567BUfCqrs4FdtYZakHIuvGV3eMylmn";

    for (size_t i = 0; i < keys.size(); ++i) {
        bt.Insert(T3(keys[i]), Ref(i * i));
    }

    size_t letterCount = 0;


    // ✔ usar ObjectInfo directamente desde el BTree
    bt.forEach([](BTree<MyTrait>::ObjectInfo& e,
                      size_t level,
                      size_t& count)
    {
        if (isalpha(e.GetData())) {
            ++count;
        }
    }, std::ref(letterCount));

    std::cout << "Letras en el arbol: " << letterCount << "\n";

    T3 target = 'Z';

    auto* entry = bt.firstThat(
        [](BTree<MyTrait>::ObjectInfo& e, size_t level, T3 t) {
            return e.GetData() == t;
        },
        target
    );

    if (entry) {
        std::cout << "firstThat('Z') -> Ref: " << entry->GetRef() << "\n";
    }

    std::string inorderKeys;

    for (auto& entry : bt) {
        inorderKeys += entry;
    }

    std::cout << "Claves en orden: " << inorderKeys << "\n";
}
