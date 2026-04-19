#include "linkedlist.h"
#include "util.h"
#include <iostream>
#include <fstream>

void LinkedListDemo(){
    LinkedList<DescendingLinkedListTrait<T1>> list;
    list.insert(1, 15);
    list.insert(2, 25);
    list.insert(3, 35);
    list.insert(4, 45);
    list.insert(5, 55);
    cout << list << endl;


    LinkedList<DescendingLinkedListTrait<string>> list2;
    list2.insert("hola", 15);
    list2.insert("mundo", 25);
    list2.insert("!", 35);
    list2.insert("!", 45);
    list2.insert("!", 55);
    list2.push_front("Inicio",0);
    list2.push_back("back",224);
    cout << list2 << endl;
    list2.pop_front();
    cout << list2 << endl;
    list2.pop_back();
    cout << list2 << endl;

    ofstream of("tempLL.txt");
    of << list << endl;
    of << list2 << endl;
    // of.close();

    LinkedList<DescendingLinkedListTrait<T1>> list3 = list;
    cout << list3 << endl;


    LinkedList<DescendingLinkedListTrait<int>> a;
    a.insert(1, 10);
    a.insert(2, 20);
    cout<<a<<endl;
    LinkedList<DescendingLinkedListTrait<int>> b = std::move(a);
    cout<<a<<endl;
    cout<<b<<endl;
}

void ListsDemo(){
    LinkedListDemo();
    
}
