#include <iostream>
#include <fstream>
#include <string>
#include <thread>      // Arregla el error de thread
#include <vector>
#include "../types.h"
#include "linkedlist.h"
#include "doublelinkedlist.h"
// #include "circularlinkedlist.h"
// #include "circularlinkedlist.h"

using namespace std;
template <typename T>
void Add(T& n, T value){
    n += value;
}

template <typename Container>
void DemoList(Container& list, string fileName){
    list.insert(28, 15);
    list.insert(17, 25);
    list.insert(8, 35);
    list.insert(4, 45);
    list.insert(35, 55);
    cout << list << endl;
    // Grabar la lista en un archivo
    ofstream os(fileName);
    os << list << endl;

    // Leer la lista desde un archivo
    ifstream is(fileName);
    is >> list;
    cout << list << endl;

    cout <<"FOR EACH ADD 5" << endl;
    list.ForEach(Add<typename Container::value_type>,5);
    cout << list << endl;

    cout <<"REVERSE FOREACH ADD 15" << endl;
    list.ReverseForEach(Add<typename Container::value_type>,15);
    cout << std::endl << std::endl;
}

void LinkedListDemo(){
    //cout <<"DEMO lINKED lIST" << std::endl << std::endl;
    //LinkedList<AscendingLinkedListTrait<T1>> list;
    //DemoList(list, "AscLL.txt");

    //LinkedList<DescendingLinkedListTrait<T1>> list2;
   // DemoList(list2, "DescLL.txt");
}


void DoubleLinkedListDemo(){
    cout <<"DEMO DOUBLE lINKED lIST" << std::endl << std::endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> list;
    DemoList(list, "AscDLL.txt");
    DoubleLinkedList<DescendingDLLTrait<T1>> list2;
    DemoList(list2, "DescDLL.txt");
    // 2. Prueba de caja blanca (estructura doble)
    cout <<"PRUEBA DE ENLACES DOUBLE lINKED lIST" << endl;
    list.printBackward(); // Si esto funciona, tu lista es realmente 'doble'
    list.verifyLinks();   // Si esto no imprime error, tu lógica de punteros es perfecta


    cout << list << endl;

}

void CircularLinkedListDemo(){
    
}
void CircularDoubleLinkedListDemo(){
}


void ListsDemo(){
    LinkedListDemo();
    //CircularLinkedListDemo();
    DoubleLinkedListDemo();
    //CircularDoubleLinkedListDemo();
}

void TestConcurrencia() {
    cout << "\nTEST DE CONCURRENCIA" << endl;

    // CORRECCIÓN: Plantilla correcta
    LinkedList<AscendingLinkedListTrait<T1>> list;

    auto worker = [&list](int thread_id) {
        for(int i = 0; i < 1000; i++) {
            list.insert(i, thread_id);
        }
    };

    // CORRECCIÓN: Los hilos se declaran y asignan correctamente
    thread t1(worker, 1);
    thread t2(worker, 2);
    thread t3(worker, 3);
    thread t4(worker, 4);
    thread t5(worker, 5);

    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();

    cout << "Se lanzaron 5 hilos insertando 1000 elementos simultaneamente." << endl;
    cout << "Tamano de la lista (Esperado 5000): " << list.size() << endl;
    if(list.size() == 5000) {
        cout << "ESTADO: EXITO - El shared_mutex previno condiciones de carrera perfectamente." << endl;
    } else {
        cout << "ESTADO: FALLO - Hubo corrupcion de memoria." << endl;
    }
}
void TestOperators() {
    cout << "\nTEST DE OPERADORES" << endl;
    LinkedList<AscendingLinkedListTrait<T1>> list;
    
    // 1. Probamos operator>> (Lectura)
    cout << "Simulando lectura desde formato: [(10, 100), (20, 200), (30, 300)]" << endl;
    stringstream simulador_input("[(10, 100), (20, 200), (30, 300)]");
    simulador_input >> list;

    // 2. Probamos operator<< (Escritura)
    cout << "Lista luego de la lectura (operator<<): " << list << endl;
    
    // 3. Probamos operator[] (Acceso seguro por indice)
    cout << "Accediendo al indice [0] (operator[]): Dato -> " << list[0] << endl;
    cout << "Accediendo al indice [2] (operator[]): Dato -> " << list[2] << endl;
    
    // Probamos la excepcion del operator[] (Descomentar para probar)
    // cout << "Probando fuera de rango: " << list[5] << endl; // Lanzara la excepcion
}
//void ListsDemo(){
    //TestBasicos();
    //TestConcurrencia();
    //TestOperators();
    //cout << "\n=== FIN DE LAS PRUEBAS ===" << endl;
//}
