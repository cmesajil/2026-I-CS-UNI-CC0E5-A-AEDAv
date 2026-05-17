#include <iostream>
#include <fstream>
#include <string>
#include <thread>      // Arregla el error de thread
#include <vector>
#include "../types.h"
#include "linkedlist.h"
#include "doublelinkedlist.h"
#include "CircularLinkedList.h"
#include "CircularDoubleLinkedList.h"
#include "BinaryTree.h"
#include "AvlBinaryTree.h"
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
/*
    cout <<"FOR EACH ADD 5" << endl;
    list.ForEach(Add<typename Container::value_type>,5);
    cout << list << endl;

    cout <<"PUSH FRONT" << endl;
    list.push_front(1,1);
    cout << list << endl;
    cout <<"PUSH BACK" << endl;
    list.push_back(9,9);
    cout << list << endl;
    cout <<"POP FRONT" << endl;
    list.pop_front();
    cout << list << endl;
    cout <<"POP BACK" << endl;
    list.pop_back();
    cout << list << endl;


*/
    //cout <<"REVERSE FOREACH ADD 15" << endl;
    //list.ReverseForEach(Add<typename Container::value_type>,15);
   // cout << std::endl << std::endl;
}

void LinkedListDemo(){
    cout <<"DEMO lINKED lIST-------------------------------" << std::endl << std::endl;
    LinkedList<AscendingLinkedListTrait<T1>> list;
    DemoList(list, "AscLL.txt");

    LinkedList<DescendingLinkedListTrait<T1>> list2;
    DemoList(list2, "DescLL.txt");
    //destructor
}


void DoubleLinkedListDemo(){
    cout <<"DEMO DOUBLE lINKED lIST----------------------------" << std::endl << std::endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> list;
    DemoList(list, "AscDLL.txt");
    DoubleLinkedList<DescendingDLLTrait<T1>> list2;
    DemoList(list2, "DescDLL.txt");
    // 2. Prueba de caja blanca (estructura doble)
    cout <<"PRUEBA DE ENLACES DOUBLE lINKED lIST" << endl;


    cout << list << endl;
    cout <<"PRUEBA DE COPY CONSTRUCTOR DOUBLE lINKED lIST" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> list3 = list;
    cout << list3 << endl;

    cout << "Forward iterator: "<< endl;
    for (auto it = list3.begin(); it != list3.end(); ++it)
        cout << *it << " ";
    cout << endl;

    cout << "Backward iterator: "<< endl;
    for (auto it = list3.rbegin(); it != list3.rend(); ++it)
        cout << *it << " ";
    cout << endl;



    cout <<"PRUEBA DE MOVE CONSTRUCTOR DOUBLE lINKED lIST" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> list4=std::move(list);
    cout << list4 << endl;


    cout << "LIST VACIO?: "<<list << endl;


    cout <<"PRUEBA DE COPY ASSIGMENT DOUBLE lINKED lIST" << endl;
    DoubleLinkedList<AscendingDLLTrait<T1>> list5;
    list5=list4;
    cout << list5 << endl;


    cout << "LIST4 VACIO?: "<<list4 << endl;

    cout <<"PRUEBA DE COPY ASSIGMENT DOUBLE lINKED lIST" << endl;
    list5=std::move(list4);;
    cout << list5 << endl;


    cout << "LIST4 VACIO?: "<<list4 << endl;

}

void CircularLinkedListDemo(){
    //insercion ordenada
    cout << "DEMO CIRCULAR LINKED LIST------------------------------" << endl<< endl;
    CircularLinkedList<AscendingCLLTrait<T1>>list;
    DemoList(list, "AscCLL.txt");
    CircularLinkedList<DescendingCLLTrait<T1>> list2;
    DemoList(list2, "DesCLL.txt");

    cout << "Forward iterator: "<< endl;
    for (auto it = list2.begin(); it != list2.end(); ++it)
        cout << *it << " ";
    cout << endl;


    cout <<"FOR EACH ADD 5" << endl;
    list.ForEach(Add<T1>,5);
    cout << list << endl;

    cout <<"PUSH FRONT" << endl;
    list.push_front(1,1);
    cout << list << endl;
    cout <<"PUSH BACK" << endl;
    list.push_back(9,9);
    cout << list << endl;
    cout <<"POP FRONT" << endl;
    list.pop_front();
    cout << list << endl;
    cout <<"POP BACK" << endl;
    list.pop_back();
    cout << list << endl;
}
void CircularDoubleLinkedListDemo(){
    cout << "DEMO CIRCULAR DOUBLE LINKED LIST----------------------------" << endl<< endl;
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>>list;
    DemoList(list, "AscCLL.txt");
    CircularDoubleLinkedList<DescendingCDLLTrait<T1>> list2;
    DemoList(list2, "DesCLL.txt");


    cout << "Forward iterator: "<< endl;
    for (auto it = list2.begin(); it != list2.end(); ++it)
        cout << *it << " ";
    cout << endl;

    cout << "Backward iterator: "<< endl;
    for (auto it = list2.rbegin(); it != list2.rend(); ++it)
        cout << *it << " ";
    cout << endl;

}
template <typename T>
void print_dfs(BinaryTree<T>& btree) {
    cout << "--- FORWARD ITERATORS ---" << endl;
    cout << "Inorder iterator: " << endl;
    btree.print_inorder();
    cout << endl;

    cout << "Forward preorder iterator: " << endl;
    btree.print_preorder();
    cout << endl;

    cout << "Forward postorder iterator: " << endl;
    btree.print_postorder();
    cout << endl;

    cout << "--- BACKWARD ITERATORS ---" << endl;
    cout << "Backward inorder iterator: " << endl;
    btree.print_reverse_inorder();
    cout << endl;

    cout << "Backward preorder iterator: " << endl;
    btree.print_reverse_preorder();
    cout << endl;

    cout << "Backward postorder iterator: " << endl;
    btree.print_reverse_postorder();
    cout << endl;
}

void BinaryTreeDemo(){
    cout << "DEMO BINARY TREE WITH FORWARD/BACKWARD ITERATORS------------" << endl << endl;

    BinaryTree<AscendingBinaryTreeTrait<T1>> asc;
    asc.insert(8, 8);
    asc.insert(3, 3);
    asc.insert(10, 10);
    asc.insert(1, 1);
    asc.insert(6, 6);

    cout << "Ascending Tree: " << asc << endl << endl;

    BinaryTree<DescendingBinaryTreeTrait<T1>> desc;
    desc.insert(8, 8);
    desc.insert(3, 3);
    desc.insert(10, 10);
    desc.insert(1, 1);
    desc.insert(6, 6);

    cout << "Descending Tree: " << desc << endl << endl;

    BinaryTree<AscendingBinaryTreeTrait<T1>> btree;
    btree.insert(8, 8);
    btree.insert(3, 3);
    btree.insert(10, 10);
    btree.insert(1, 1);
    btree.insert(6, 6);
    btree.insert(14, 14);
    btree.insert(4, 4);
    btree.insert(7, 7);

    cout << "Ascending Tree: " << btree << endl << endl;

    // Prueba manual de Forward Inorder
    cout << "Forward in order iterator (Manual loop): "<< endl;
    for (auto it = btree.begin(); it != btree.end(); ++it)
        cout << *it << " ";
    cout << endl << endl;

    // PRUEBA NUEVA: Prueba manual de Backward Inorder
    cout << "Backward in order iterator (Manual loop): "<< endl;
    for (auto it = btree.rbegin(); it != btree.rend(); ++it)
        cout << *it << " ";
    cout << endl << endl;

    // Llamada a la función DFS que probará el resto de forwards y backwards
    print_dfs(btree);
    cout << "Graban el árbol en binarytree.txt y luego lo lee de vuelta" << endl;
    // Grabar la lista en un archivo
    ofstream os("binarytree.txt");
    os << btree << endl;

    // Leer la lista desde un archivo
    ifstream is("binarytree.txt");
    is >> btree;
    cout << btree << endl;
    //copy constructor
    cout << "copy constructor" << endl;
    BinaryTree<AscendingBinaryTreeTrait<T1>> btree2(btree);
    cout << btree2 << endl;
    // move constructor
    cout << "Move constructor" << endl;
    BinaryTree<AscendingBinaryTreeTrait<T1>> btree3(std::move(btree));
    cout << btree3 << endl;
    // destructor
    btree2.~BinaryTree();
    btree3.~BinaryTree();

    cout << btree2 << endl;
    cout << btree3 << endl;
}

void AvlBinaryTree(){
    std::cout << "--- Inicializando AVL Genérico por Traits ---" << std::endl;

        // Instanciamos el árbol usando el Trait abstracto
        AVLTree<AVLAscendingTrait<T1>> btree;

        // Insertamos los datos de tu ejemplo anterior.
        // Recuerda que un BST normal se desbalancearía al meter la secuencia,
        // pero el AVL va a disparar rotaciones automáticas para mantenerse optimizado.
        btree.insert(8, 8);
        btree.insert(3, 3);
        btree.insert(10, 10);
        btree.insert(1, 1);
        btree.insert(6, 6);
        btree.insert(14, 14);
        btree.insert(4, 4);
        btree.insert(7, 7);
        btree.insert(5, 5);

        std::cout << "Recorrido Inorder del AVL (Formato -> (Dato, H:Altura)): " << std::endl;
        // Llamada a la función DFS que probará el resto de forwards y backwards
        print_dfs(btree);

}


void ListsDemo(){
    BinaryTreeDemo();
    AvlBinaryTree();
    //LinkedListDemo();
    //CircularLinkedListDemo();
    //DoubleLinkedListDemo();
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
