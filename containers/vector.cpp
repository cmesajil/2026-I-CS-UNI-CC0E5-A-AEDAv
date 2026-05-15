#include <algorithm>
#include <cstddef>
#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include "vector.h"
#include "heap.h"
using namespace std;

void AddOne(int& n){
    static mutex mtx;
    scoped_lock lock(mtx);
    ++n;
}

template <typename T>
void Add(T& n, T value){
    n += value;
}

void DemoVector(){
    Vector<AscendingVectorTrait<T1>> v1(3);
    v1.push_back(1, 11);
    v1.push_back(2, 22);
    v1.push_back(-1, -15);
    v1.push_back(4, 45);
    cout << v1.toString() << endl;
    cout << v1 << endl;
    // cout << "hola" << 5 << endl;
    // cout.operator<<("hola")
    // ==============
    //           cout << 5 << endl;
    //           =========
    //                cout << endl;

    Vector<AscendingVectorTrait<T2>> v2(10);
    v2.push_back("Hola", 5);
    v2.push_back("Mundo", 6);
    v2.push_back("!", 1);
    cout << v2 << endl;
    cout << v2.toString() << endl;

    ofstream of("temp.txt");
    of << v1 << endl;
    of << v2 << endl;
    // of.close();

    ForEach(v1, AddOne);
    Print(v1, cout);
    ForEach(v1, Add<T1>, 10);
    cout << "Imprimiendo desde adentro de Vector:" << endl;
    v1.ForEach(Add<T1>, 5);
    Print(v1, cout);
    v1.ForEach(PrintX<T1>, cout, ", ");
    cout << endl;
    v1.ReverseForEach(PrintX<T1>, cout, ", ");
    cout << endl;

    Print(v2, cout);
    ForEach(v2, Add<T2>, string("XYZ"));
    cout << v2 << endl;

    Print(v1, of);
    Print(v2, of);
}
void DemoHeap(){

    std::cout << "=== DEMOSTRACIÓN DE HEAP CON TRAITS Y CONCURRENCIA ===\n\n"<< endl;

    Heap<MinHeapTrait<T1>> heap(10);
    heap.insert(16, 16);
    heap.insert(28, 28);
    heap.insert(71, 71);
    heap.insert(7, 7);
    heap.insert(13, 13);
    heap.insert(3, 3);

    std::cout << "[MinHeap] Vector interno (debería priorizar menores en la raíz):\n";
    std::cout << heap << "\n"; // Llama al nuevo operator<< del HeapNode
    std::cout << "Peek mínimo: " << heap.peek().getData() << "\n\n";

    Heap<MaxHeapTrait<T1>> maxheap(3);
    maxheap.insert(16, 16);
    maxheap.insert(28, 28);
    maxheap.insert(71, 71);
    maxheap.insert(7, 7);
    maxheap.insert(13, 13);
    maxheap.insert(3, 3);
    std::cout << "[MaxHeap] Vector interno (debería priorizar mayores en la raíz):\n";
    std::cout << maxheap << "\n";
    std::cout << "Peek máximo: " << maxheap.peek().getData() << "\n";
    std::cout << "\n======================================================\n";


    //  Grabar la lista en un archivo
    ofstream os("heap.txt");
    os << heap << endl;
    os.close();
    // Leer la lista desde un archivo
    ifstream is("heap.txt");
    Heap<MinHeapTrait<T1>> readheap(10);
    is >> readheap;
    cout << readheap << endl;
    is.close();
}
// DemoConcurrentVector
void DemoConcurrentVector(){
    Vector<AscendingVectorTrait<T1>> v(4);
    v.push_back(0, 0);
    v.push_back(0, 0);
    v.push_back(0, 0);
    v.push_back(0, 0);

    // Cada thread itera el vector 100,000 veces e incrementa cada elemento
    // Sin sincronizacion → race condition en los contadores
    auto worker = [&v](int thread_id){
        for(int i = 0; i < 100000; i++)
            v.ForEach(AddOne);
        cout << "Thread " << thread_id << " terminado\n";
    };

    thread t1(worker, 1);
    thread t2(worker, 2);
    thread t3(worker, 3);
    thread t4(worker, 4);
    thread t5(worker, 5);

    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();

    // Resultado esperado sin race condition: 4 elementos * 100000 * 5 threads = 500000
    cout << "Resultado (esperado 500000): " << v << endl;
}
