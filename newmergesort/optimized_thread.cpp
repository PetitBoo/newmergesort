#include "optimized_thread.h"
OptimizedThreadPool::OptimizedThreadPool() :
    m_thread_count(thread::hardware_concurrency() != 0 ? thread::hardware_concurrency() : 4),
    //thread::hardware_concurrency() - возвращает количество ядер. Если же thread::hardware_concurrency() возвращает 0, то устанавливает на 4
    m_thread_queues(m_thread_count) {}


void OptimizedThreadPool::start() {
    for (int i = 0; i < m_thread_count; i++) {//заполняем массив с потоками(4)
        m_threads.emplace_back(&OptimizedThreadPool::threadFunc, this, i);//this это this->threadFunc в потоке
    }
};

void OptimizedThreadPool::stop() {//для остановки потоков
    for (int i = 0; i < m_thread_count; i++) {
        // кладем задачу-пустышку в каждую очередь
        // для завершения потока
        task_type empty_task;
        m_thread_queues[i].push(empty_task);
    }
    for (auto& t : m_threads) {//Все потоки в пуле m_threads выходят из режима ожидания и завершаются если очередь задач пуста
        t.join();
    }
};

void OptimizedThreadPool::push_task(FuncType func, std::vector<int> &vec, int a, int b) {
    // вычисляем индекс очереди, куда положим задачу
    int queue_to_push = m_qindex++ % m_thread_count;
    task_type new_task([&vec, a, b, func] {func(vec, a, b); });//создается функтор с заданной функцией, переданной через лямду
    m_thread_queues[queue_to_push].push(new_task);//new_task кладется в очередь,последняя хранится в классе(private)
};

void OptimizedThreadPool::threadFunc(int qindex) {//quindex - индекс очереди
    while (true) //бесконечный цикл, который находится в режиме ожидания, пока не придёт оповещение.Как только поступает новая задача на выполнение, поток просыпается, забирает её оттуда и выполняет задачу
    {
        task_type task_to_do;//задача для выполнения
        bool res;//удалось получить при помощи fastpop элемент очереди
        int i = 0;
        for (i; i < m_thread_count; i++) {
            // попытка быстро забрать задачу из любой очереди, начиная со своей
            if (res = m_thread_queues[(qindex + i) % m_thread_count].fast_pop(task_to_do))
                break;
        }

        if (!res) {
            // вызываем блокирующее получение очереди
            m_thread_queues[qindex].pop(task_to_do);
        }
        else if (!task_to_do) {
            // чтобы не допустить зависания потока
            // кладем обратно задачу-пустышку
            m_thread_queues[(qindex + i) % m_thread_count].push(task_to_do);
        }

        if (!task_to_do) {
            return;
        }
        // выполняем задачу
        task_to_do();


    }

};

RequestHandler2::RequestHandler2() {//конструктор,который запускает пул
    m_tpool.start();
}
RequestHandler2::~RequestHandler2() {//деструктор,который останавливает пул
    m_tpool.stop();
}
void RequestHandler2::push_task(FuncType func, std::vector<int>& vec, int a, int b) {
    m_tpool.push_task(func, vec, a, b);
}
