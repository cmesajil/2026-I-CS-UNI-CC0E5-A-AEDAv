#include <algorithm>
#include <cstddef>
#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include "vector.h"
#include "heap.h"
#include "util.h"
using namespace std;

void AddOne(T1& n){
    static mutex mtx;
    scoped_lock lock(mtx);
    ++n;
}

template <typename T>
void Add(T& n, T value){
    n += value;
}

void DemoVector(){
    // Usamos Ref() o constructores explícitos para capacidades
    Vector<AscendingVectorTrait<T1>> v1(3);
    v1.push_back(T1(1), Ref(11));
    v1.push_back(T1(2), Ref(22));
    v1.push_back(T1(-1), Ref(-15));
    v1.push_back(T1(4), Ref(45));

    cout << v1.toString() << endl;
    cout << v1 << endl;

    Vector<AscendingVectorTrait<T2>> v2(10);
    v2.push_back(T2("Hola"), Ref(5));
    v2.push_back(T2("Mundo"), Ref(6));
    v2.push_back(T2("!"), Ref(1));
    cout << v2 << endl;
    cout << v2.toString() << endl;

    ofstream of("temp.txt");
    of << v1 << endl;
    of << v2 << endl;

    ForEach(v1, AddOne);
    Print(v1, cout);

    // CORREGIDO: Pasamos T1(10) y T1(5) en lugar de números nativos sueltos
    ForEach(v1, Add<T1>, T1(10));
    cout << "Imprimiendo desde adentro de Vector:" << endl;
    v1.ForEach(Add<T1>, T1(5));

    Print(v1, cout);
    v1.ForEach(PrintX<T1>, cout, ", ");
    cout << endl;
    v1.ReverseForEach(PrintX<T1>, cout, ", ");
    cout << endl;

    Print(v2, cout);
    ForEach(v2, Add<T2>, T2("XYZ"));
    cout << v2 << endl;

    Print(v1, of);
    Print(v2, of);

    std::cout << "Haciendo pop_back y obteniendo nodo :\n";
    std::cout << v1.pop_back_unsafe()<< "\n";
    std::cout << v1 << endl;
}

void DemoHeap(){
    std::cout << "=== DEMOSTRACIÓN DE HEAP CON TRAITS Y CONCURRENCIA ===\n\n"<< endl;

    Heap<MinHeapTrait<T1>> heap(10);
    heap.insert(T1(16), Ref(16));
    heap.insert(T1(28), Ref(28));
    heap.insert(T1(71), Ref(71));
    heap.insert(T1(7), Ref(7));
    heap.insert(T1(13), Ref(13));
    heap.insert(T1(3), Ref(3));

    std::cout << "[MinHeap] Vector interno (debería priorizar menores en la raíz):\n";
    std::cout << heap << "\n";
    std::cout << "Peek mínimo: " << heap.peek().getData() << "\n\n";

    std::cout << "Haciendo extract y obteniendo nodo :\n";
    std::cout << heap.extract()<< "\n";
    std::cout << heap << endl;

    Heap<MaxHeapTrait<T1>> maxheap(3);
    maxheap.insert(T1(16), Ref(16));
    maxheap.insert(T1(28), Ref(28));
    maxheap.insert(T1(71), Ref(71));
    maxheap.insert(T1(7), Ref(7));
    maxheap.insert(T1(13), Ref(13));
    maxheap.insert(T1(3), Ref(3));

    std::cout << "[MaxHeap] Vector interno (debería priorizar mayores en la raíz):\n";
    std::cout << maxheap << "\n";
    std::cout << "Peek máximo: " << maxheap.peek().getData() << "\n";
    std::cout << "\n======================================================\n";

    ofstream os("heap.txt");
    os << heap << endl;
    os.close();

    ifstream is("heap.txt");
    Heap<MinHeapTrait<T1>> readheap(10);
    is >> readheap;
    cout << readheap << endl;
    is.close();
}

void DemoConcurrentVector(){
    Vector<AscendingVectorTrait<T1>> v(4);
    v.push_back(T1(0), Ref(0));
    v.push_back(T1(0), Ref(0));
    v.push_back(T1(0), Ref(0));
    v.push_back(T1(0), Ref(0));

    // Definimos el límite usando una constante de tipo estándar de conteo
    const size_t max_iteraciones = 100000;

    auto worker = [&v, max_iteraciones](T1 thread_id){
        for (size_t i = 0; i < max_iteraciones; ++i) {
            v.ForEach(AddOne);
        }
        cout << "Thread " << thread_id << " terminado\n";
    };

    // CORREGIDO: Los hilos se inicializan mandando objetos T1 explícitos
    thread t1(worker, T1(1));
    thread t2(worker, T1(2));
    thread t3(worker, T1(3));
    thread t4(worker, T1(4));
    thread t5(worker, T1(5));

    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();

    cout << "Resultado (esperado 500000): " << v << endl;
}
