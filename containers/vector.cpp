#include "vector.h"
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

using namespace std;

void AddOne(int &n) { ++n; }

template <typename T> void Add(T &n, T value) { n += value; }

void DemoVector() {
  Vector<T1> v1(3);
  v1.push_back(1, 11);
  v1.push_back(2, 22);
  v1.push_back(-1, -15);
  v1.push_back(4, 45);

  // cin >> v1;
  // cout << v1.toString() << endl;
  cout << v1 << endl;
  // cout << "hola" << 5 << endl;
  // cout.operator<<("hola")
  // ==============
  //           cout << 5 << endl;
  //           =========
  //                cout << endl;
  Vector<string> v2(10);
  v2.push_back("Hola", 5);
  v2.push_back("Mundo", 6);
  v2.push_back("!", 1);

  // cin >> v2;

  cout << v2 << endl;
  cout << v2.toString() << endl;

  ofstream of("temp.txt");
  of << v2 << endl;
  of.close();

  // Probar operator>>
  ifstream ifs("temp.txt");
  Vector<string> v3(10);
  ifs >> v3;
  cout << "Leido del archivo temp.txt v2: " << v3 << endl;
  ifs.close();
  cout << v3 << endl;

  ofstream of2("temp2.txt");
  of2 << v1 << endl;
  // of.close();

  // Probar operator>>
  ifstream ifs2("temp2.txt");
  Vector<T1> v4(10);
  ifs2 >> v4;
  cout << "Leido del archivo temp2.txt v1: " << v4 << endl;
  ifs.close();
  cout << v4 << endl;

  /*
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
    ForEach(v2, Add<string>, string("XYZ"));
    cout << v2 << endl;

    Print(v1, of);
    Print(v2, of);
    */
}

// DemoConcurrentVector
void DemoConcurrentVector() {
  Vector<T1> v(4);
  v.push_back(0, 0);
  v.push_back(0, 0);
  v.push_back(0, 0);
  v.push_back(0, 0);

  // Cada thread itera el vector 100,000 veces e incrementa cada elemento
  // Sin sincronizacion → race condition en los contadores
  auto worker = [&v](int thread_id) {
    for (int i = 0; i < 100000; i++)
      v.ForEach(AddOne);
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
  cout << "Resultado (esperado 500000): " << v << endl;
}