#include <iostream>
#include <thread>

using namespace std;

static const int num_threads = 2;

//This function will be called from a thread
 
void call_from_thread(int i) {
    cout << "Launched by thread " << i << endl; 
}
 
int main() {
    thread t[num_threads];
    
    for (int i = 0; i < num_threads; ++i) {
        t[i] = std::thread(call_from_thread, i);
    }
 
    cout << "Launched from the main" << endl;


    for (int i = 0; i < num_threads; ++i) {
        t[i].join();
    }
 
    return 0;
}
