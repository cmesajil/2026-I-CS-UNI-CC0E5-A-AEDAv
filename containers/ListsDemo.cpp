#include <iostream>
#include <fstream>
#include <string>

#include "../types.h"
#include "linkedlist.h"
// #include "doublelinkedlist.h"
// #include "circularlinkedlist.h"
// #include "circularlinkedlist.h"

using namespace std;

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
}

void LinkedListDemo(){
    LinkedList<T1, AscendingLinkedListTrait<T1>> list;
    DemoList(list, "AscLL.txt");
    LinkedList<T1, DescendingLinkedListTrait<T1>> list2;
    DemoList(list2, "DescLL.txt");
}

void DoubleLinkedListDemo(){
    // DoubleLinkedList<T1, AscendingDLLTrait<T1>> list;
    // DemoList(list, "AscDLL.txt");
    // DoubleLinkedList<T1, DescendingDLLTrait<T1>> list2;
    // DemoList(list2, "DescDLL.txt");
}

void CircularLinkedListDemo(){
    
}

void CircularLinkedListDemo(){
    
}

void ListsDemo(){
    LinkedListDemo();
    CircularLinkedListDemo();
    DoubleLinkedListDemo();
    CircularDoubleLinkedListDemo();
}
