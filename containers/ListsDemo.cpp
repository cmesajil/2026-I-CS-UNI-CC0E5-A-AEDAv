#include "linkedlist.h"
#include <fstream>
#include <iostream>
#include <thread>

template <typename T> void Add(T &n, T value) { n += value; }

void LinkedListDemo() {
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
  list2.push_front("Inicio", 0);
  list2.push_back("back", 224);
  cout << list2 << endl;
  list2.pop_front();
  cout << list2 << endl;
  list2.pop_back();
  cout << list2 << endl;

  ofstream of("tempLL.txt");
  of << list << endl;
  // of << list2 << endl;
  //  of.close();

  LinkedList<DescendingLinkedListTrait<T1>> list3 = list;
  cout << list3 << endl;

  LinkedList<DescendingLinkedListTrait<int>> a;
  a.insert(1, 10);
  a.insert(2, 20);
  cout << a << endl;
  LinkedList<DescendingLinkedListTrait<int>> b = std::move(a);
  cout << a << endl;
  cout << b << endl;

  list2.ForEach(Add<string>, string("XYZ"));
  cout << list2 << endl;

  cout << "Buscando el valor con ref 25: " << list2[25] << endl;

  ifstream is("tempLL.txt");
  LinkedList<DescendingLinkedListTrait<T1>> list4;
  is >> list4;
  cout << list4 << endl;
}

void DemoConcurrentLinkedList() {
  LinkedList<DescendingLinkedListTrait<int>> list;
  list.insert(0, 15);
  list.insert(0, 25);
  list.insert(0, 35);
  list.insert(0, 45);
  list.insert(0, 55);
  cout << list << endl;
  // Cada thread itera el vector 100,000 veces e incrementa cada elemento
  // Sin sincronizacion → race condition en los contadores
  auto worker = [&list](int thread_id) {
    for (int i = 0; i < 100000; i++)
      list.ForEach(Add<T1>, 1);
    cout << "Thread " << thread_id << " terminado\n";
  };

  thread t1(worker, 1);
  thread t2(worker, 2);
  thread t3(worker, 3);
  thread t4(worker, 4);
  thread t5(worker, 5);

  t1.join();
  t2.join();
  t3.join();
  t4.join();
  t5.join();

  // Resultado esperado sin race condition: 4 elementos * 100000 * 5 threads =
  // 500000
  cout << "Resultado (esperado 500000): " << list << endl;

  // concurrencia de push pop , ... etc ,la idea es que salga un lista enlazada
  // decidida
  // LinkedList<DescendingLinkedListTrait<int>> list2;
  // list2.insert(0, 15);
}

void ListsDemo() {
  LinkedListDemo();
  DemoConcurrentLinkedList();
}
