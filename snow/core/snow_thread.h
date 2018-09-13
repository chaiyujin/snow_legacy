#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

namespace snow {

template <class T>
class SafeQueue {
private:
    size_t                  mSize;
    std::queue<T>           mQueue;
    mutable std::mutex      mMutex;
    std::condition_variable mCondVar;

public:
    SafeQueue(void) : mQueue(), mMutex(), mCondVar(), mSize(0) {}
    ~SafeQueue(void) {}

    // Add an element to the queue.
    void push(T t) {
        std::lock_guard<std::mutex> lock(mMutex);
        mQueue.push(t);
        mSize++;
        mCondVar.notify_one();
    }

    // Get the "front"-element.
    // If the queue is empty, wait till a element is avaiable.
    void pop(void) {
        std::unique_lock<std::mutex> lock(mMutex);
        // release lock as long as the wait and reaquire it afterwards.
        while (mQueue.empty())
            mCondVar.wait(lock);
        mQueue.pop();
        mSize--;
    }

    T &front() {
        std::unique_lock<std::mutex> lock(mMutex);
        // release lock as long as the wait and reaquire it afterwards.
        while (mQueue.empty())
            mCondVar.wait(lock);
        return mQueue.front();
    }

    size_t size() const { return mSize; }

};

}