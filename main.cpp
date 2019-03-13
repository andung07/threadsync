#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <condition_variable>
#include <mutex>

class workerThread {
public:
    workerThread(std::string name, int sleep, std::mutex* m, std::condition_variable* cv) :
        _thread(&workerThread::threadFunction, this), _sleep(sleep) {

        ready = false;
        _name = name;
        _m = m;
        _cv = cv;
    }

    void threadFunction() {
        while (true) {
            std::unique_lock<std::mutex> lk(*_m);
            _cv->wait(lk, [this]{return ready;});

            std::cout << _name << " STARTING" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(_sleep));
            std::cout << _name << " DONE" << std::endl;

            ready = false;

            lk.unlock();
            _cv->notify_one();
        }
    }

    void join() {
        _thread.join();
    }

    bool ready;

private:
    std::thread _thread;
    std::string _name;
    int _sleep;
    std::mutex* _m;
    std::condition_variable* _cv;
};

int main() {

    std::mutex m1, m2, m3;
    std::condition_variable cv1, cv2, cv3;

    workerThread wThread1("wThread1", 1, &m1, &cv1);
    workerThread wThread2("wThread2", 1, &m2, &cv2);
    workerThread wThread3("wThread3", 2, &m3, &cv3);

    while (true) {
        std::unique_lock<std::mutex> lk1(m1);
        wThread1.ready = true;
        lk1.unlock();
        cv1.notify_all();

        std::unique_lock<std::mutex> lk2(m2);
        wThread2.ready = true;
        lk2.unlock();
        cv2.notify_all();

        std::unique_lock<std::mutex> lk3(m3);
        wThread3.ready = true;
        lk3.unlock();
        cv3.notify_all();

        lk1.lock();
        cv1.wait(lk1, [&wThread1]{return !wThread1.ready;});
        lk1.unlock();

        lk2.lock();
        cv2.wait(lk2, [&wThread2]{return !wThread2.ready;});
        lk2.unlock();

        lk3.lock();
        cv3.wait(lk3, [&wThread3]{return !wThread3.ready;});
        lk3.unlock();

        std::cout << "ALL WORKER COMPLETED" << std::endl;
    }


    return 0;
}