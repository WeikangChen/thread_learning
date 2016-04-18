#include <iostream>
#include <stack>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
using namespace std;

mutex g_mtxPrint;

class Person
{
private:
    int age;
    const char* pName;
    
public:
    Person(): pName(0),age(0){}
    Person(const char* pName, int age): pName(pName), age(age) {}
    void Display() {printf("Name = %s, Age = %d \n", pName, age);}
};

class reference {
private:
    unsigned int count;
public:
    reference(unsigned int cnt = 1): count(cnt){}

    void add_count() {
        count++;
    }

    unsigned int release() {
        count --;
        return count;
    }

    unsigned int get_count() {
        return count;
    }
};


template<class T>
class smart_ptr {
private:
    T *m_pData;
    reference *m_pRef;
public:
    smart_ptr (T *ptr=NULL): m_pData(ptr) {
        m_pRef = new reference();
    }

    smart_ptr(const smart_ptr& rhs) {
        m_pData = rhs.m_pData;
        m_pRef = rhs.m_pRef;
        m_pRef->add_count();
    }

    T& operator*() {
        return *m_pData;
    }

    T* operator->() {
        return m_pData;
    }

    ~smart_ptr() {
        if(m_pRef->release() == 0) {
            delete m_pData;
            delete m_pRef;
        }
    }

    void PrintReferenceCount() {
        cout << "Print reference count = " << m_pRef->get_count() << endl;
    }
};


template<class T>
class Stack {
private:
    stack<T> m_stk;
    mutex m_mtx;
    
public:
    Stack() {}
    void push(const T& value) {
        unique_lock<mutex> locker(m_mtx);
        m_stk.push(value);
    }
    
    void pop(T &item) {
        unique_lock<mutex> locker(m_mtx);
        if(m_stk.empty()) return;
        item = m_stk.top();
        m_stk.pop();
    }
    
    smart_ptr<T> pop(){
        unique_lock<mutex> locker(m_mtx);
        if(m_stk.empty()) return;
        smart_ptr<T> res = new T(m_stk.top());
        return res;
    }
};

template<class T>
class ThreadStack {
private:
    stack<T> m_stk;
    mutex m_mtx;
    condition_variable m_convar;
    
public:
    ThreadStack() {}
    void push(const T& value) {
        unique_lock<mutex> locker(m_mtx);
        m_stk.push(value);
        m_convar.notify_one();
    }
    
    void pop(T &item) {
        unique_lock<mutex> locker(m_mtx);
        while(m_stk.empty()) m_convar.wait(locker);
        item = m_stk.top();
        m_stk.pop();
    }
    
    smart_ptr<T> pop(){
        unique_lock<mutex> locker(m_mtx);
        while(m_stk.empty()) m_convar.wait(locker);
        smart_ptr<T> res = new T(m_stk.top());
        return res;
    }
};

void fun_producer(int id, ThreadStack<int> &warehouse) {
    warehouse.push(id);
    {
        unique_lock<mutex> locker(g_mtxPrint);
        cout << "[producer] Add to warehouse:" << id << endl;
    }
}

void fun_consumer(int id, ThreadStack<int> &warehouse) {
    int prod = -1;
    warehouse.pop(prod);
    {
        unique_lock<mutex> locker(g_mtxPrint);
        cout << "[consumer] Get from warehouse:" << prod << endl;
    }
}



int main(int argc, char ** argv)
{
    smart_ptr<Person> p0 = new Person("kang", 33);
    smart_ptr<Person> p1 = p0;
    p0->Display();
    p1.PrintReferenceCount();


    const int n = 3;
    thread consumers[n];
    thread producers[n];

    ThreadStack<int> warehouse;
    for(int i = 0; i < n; ++i) {
        producers[i] = thread(fun_producer, 100+i, ref(warehouse));
        consumers[i] = thread(fun_consumer, 200+i, ref(warehouse));
    }

    for(int i = 0; i < n; ++i) {
        producers[i].join();
        consumers[i].join();
    }
    
    return 0;
}
