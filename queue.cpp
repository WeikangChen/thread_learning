#include <unistd.h>
#include <iostream>
#include <vector>
#include <deque>
#include <thread>
using namespace std;

mutex g_mtxPrint;

template<class T>
class queue {
private:
    deque<T> m_arr;
    mutex m_mtxQueue;
    condition_variable check_stock;
public:
    void push(const T &value) {
        unique_lock<mutex> locker(m_mtxQueue);
        m_arr.push_back(value);
        locker.unlock();
        {
            unique_lock<mutex> locker(g_mtxPrint);
            cout << "[warehouse] add " << value << " <---- [producer]" << endl;
        }
        check_stock.notify_one();
    }
    void pop() {
        unique_lock<mutex> locker(m_mtxQueue);
        while(empty()) {
            cout << "[warehouse] Nothing in stock, Please wait!" << endl;
            check_stock.wait(locker);
            cout << "[warehouse] Stock back up, take one please" << endl;
        }
        {
            unique_lock<mutex> locker(g_mtxPrint);
            cout << "[warehouse] take out " << m_arr.front() << "----> [consumer]" << endl;
        }
        m_arr.pop_front();
    }

    void pop(T &item) {
        unique_lock<mutex> locker(m_mtxQueue);
        while(empty()) {
            {
                unique_lock<mutex> locker(g_mtxPrint);
                cout << "[warehouse] Nothing in stock, Please wait!" << endl;
            }
            check_stock.wait(locker);
            {
                unique_lock<mutex> locker(g_mtxPrint);
                cout << "[warehouse] Stock back up, take one please" << endl;
            }
        }
        {
            unique_lock<mutex> locker(g_mtxPrint);
            cout << "[warehouse] take out" << m_arr.front() << "---->" << endl;
        }
        item = m_arr.front();
        m_arr.pop_front();
    }
    
    T& front() {
        unique_lock<mutex> locker(m_mtxQueue);
        while(empty()) {
            {
                unique_lock<mutex> locker(g_mtxPrint);
                cout << "[warehouse] Nothing in stock, Please wait!" << endl;
            }
            check_stock.wait(locker);
            {
                unique_lock<mutex> locker(g_mtxPrint);
                cout << "[warehouse] Stock back up, take one please" << endl;
            }
        }        
        return m_arr.front();
    }
    bool empty() {
        return m_arr.empty();
    }
    
};

void producer(int it, queue<int> &warehouse) {
    {
        unique_lock<mutex> locker(g_mtxPrint);
        cout << "[producer] add " << it << " ----> [warehouse] " << endl;
    }
    warehouse.push(it);
}

void consumer(int it, queue<int> &warehouse) {
    //this_thread::sleep_for(chrono::milliseconds(50));
    int prod = -1;
    warehouse.pop(prod);
    {
        unique_lock<mutex> locker(g_mtxPrint);
        cout << "[consumer] get " << prod << "<---- [warehouse] " << endl;
    }

}

    
int main(int argc, char ** argv)
{
    queue<int> warehouse;
    const int n = 3;
    thread prod[n];
    thread cons[n];

    for(int i = 0; i < n; ++i) {
        prod[i] = thread(producer, 100+i, ref(warehouse));
        cons[i] = thread(consumer, 200+i, ref(warehouse));
    }
    
    for(int i = 0; i < n; ++i) {
        prod[i].join();
        cons[i].join();
    }
    
    return 0;
}
