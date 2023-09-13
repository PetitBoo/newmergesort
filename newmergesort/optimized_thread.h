#pragma once
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <thread>
using namespace std;

typedef function<void()> task_type;// удобное определение для сокращения кода,[подобные функции хранятся в очереди задач,task_type - название типа
typedef void (*FuncType) (std::vector<int>&, int, int);// тип указатель на функцию, которая является эталоном(шаблоном) для функций задач,т.е. функции в потоке могут быть только void и принимать в работу только (int,int). По сути typedef function<void()> task_type передает внутри себя typedef void (*FuncType) (int, int) в void push_task(FuncType f, int id, int arg);

template<class T>
class BlockedQueue {
public:
    void push(T& item) {//— внутри этого метода блокируется мьютекс и элемент добавляется в очередь m_task_queue. После этого вызывается m_notifier.notify_one().  Этот вызов разбудит поток, который вызвал pop(), когда элементов не было в очереди.
        lock_guard<mutex> l(m_locker);
        // обычный потокобезопасный push
        m_task_queue.push(item);
        // делаем оповещение, чтобы поток, вызвавший
        // pop проснулся и забрал элемент из очереди
        m_notifier.notify_one();
    }
    // блокирующий метод получения элемента из очереди
    void pop(T& item) {//— в этом методе блокируется мьютекс, затем очередь проверяется на наличие элементов. Если таковых нет, то вызывается m_notifier.wait(). Поток, вызвавший этот метод, блокируется, пока какой-нибудь другой поток не вызовет push().
        unique_lock<mutex> l(m_locker);
        if (m_task_queue.empty())
            // ждем, пока вызовут push
            m_notifier.wait(l, [this] {return !m_task_queue.empty(); });
        item = m_task_queue.front();
        m_task_queue.pop();
    }
    // неблокирующий метод получения элемента из очереди
    // возвращает false, если очередь пуста
    bool fast_pop(T& item) {//— это метод, который будет использоваться потоками из пула, чтобы попытаться заполучить элемент из чужой очереди. В отличие от метода pop() он не блокируется, если в очереди ничего нет. Этот метод проверяет, не является ли очередь пустой, и достает оттуда элемент. Возвращает результат того, получилось ли что-то достать из очереди.
        lock_guard<mutex> l(m_locker);
        if (m_task_queue.empty())
            // просто выходим
            return false;
        // забираем элемент
        item = m_task_queue.front();
        m_task_queue.pop();
        return true;
    }
private:
    queue<T> m_task_queue;
    // уведомитель
    mutex m_locker;
    // очередь задач
    condition_variable m_notifier;
};

class OptimizedThreadPool {// пул потоков
public:
    OptimizedThreadPool();
    // запуск
    void start();
    // остановка
    void stop();
    // проброс задач типа void (*FuncType) (int, int)
    void push_task(FuncType func, std::vector<int>& vec, int a, int b);
    // функция входа для потока.
    void threadFunc(int qindex);
private:
    // количество потоков
    int m_thread_count;
    // потоки/вектор потоков
    vector<thread> m_threads;
    vector<BlockedQueue<task_type>> m_thread_queues;
    // вектор очередей m_thread_queues для каждого потока. Для равномерного распределения задач
    unsigned m_qindex;//счетчик в какую очередь добавили задачу
};

class RequestHandler2 {//менеджер потоков
public:
    RequestHandler2();
    ~RequestHandler2();
    // отправка запроса на выполнение
    void push_task(FuncType func, std::vector<int>& vec, int a, int b);//дублирует push_task
private:
    // пул потоков
    OptimizedThreadPool m_tpool;
};
