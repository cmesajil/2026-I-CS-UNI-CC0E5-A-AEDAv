void DemoBTree() {
    // Definimos BT usando tu Trait con el tipo T3 de tu proyecto
    using MyTrait = Tree23TraitAscending<T3>;
    BTree<MyTrait> bt;

    // 1. Inserción (Asumiendo que insert ya acepta T3 y Ref)
    const string keys = "D1XJ2xTg8zKL9AhijOPQcEowRSp0NbW567BUfCqrs4FdtYZakHIuvGV3eMylmn";
    for (size_t i = 0; i < keys.size(); ++i) {
        bt.insert(T3(keys[i]), Ref(i * i));
    }

    // 2. ForEach Variadic usando size_t para el nivel
    size_t letterCount = 0;
    bt.forEach([](BTree<MyTrait>::Entry& e, size_t level, size_t& count) {
        if (isalpha(e.GetData())) {
            ++count;
        }
    }, letterCount);
    cout << "Letras en el arbol: " << letterCount << "\n";

    // 3. FirstThat Variadic buscando un objetivo tipo T3
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
        inorderKeys += entry.GetData();
    }
    cout << "Claves en orden: " << inorderKeys << "\n";
}
