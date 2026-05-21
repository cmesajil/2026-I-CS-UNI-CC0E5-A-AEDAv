#include "containers/vector.h"
#include "containers/linkedlist.h"
#include "containers/heap.h"
// g++ -std=c++2b main.cpp containers/vector.cpp -o main
void ListsDemo();
int main(){
    DemoVector();
    DemoHeap();
    //DemoConcurrentVector();
    //ListsDemo();
    return 0;
}
