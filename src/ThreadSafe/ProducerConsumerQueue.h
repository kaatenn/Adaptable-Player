//
// Created by 86137 on 2023/12/22.
//

#ifndef ADAPTABLE_UPLOADER_PRODUCERCONSUMERQUEUE_H
#define ADAPTABLE_UPLOADER_PRODUCERCONSUMERQUEUE_H

#include "queue"
#include "mutex"
#include "condition_variable"
#include "string"

using std::mutex, std::queue, std::condition_variable, std::string, std::lock_guard, std::optional, std::nullopt,
std::unique_lock;
template<typename T>
class ProducerConsumerQueue {
public:
    void push(T&& t_value);

    std::optional<T> try_pop();

    std::optional<T> wait_and_pop();
private:
    mutable mutex mutex_;
    queue<T> queue_;
    condition_variable condition_variable_; // used for wait and notify
};

template<typename T>
void ProducerConsumerQueue<T>::push(T &&t_value) {
    lock_guard<mutex> lock(mutex_); // try to lock mutex, if failed, wait until mutex is unlocked
    queue_.push(std::move(t_value));
    condition_variable_.notify_one(); // notify one thread that is waiting for condition_variable
}

template<typename T>
optional<T> ProducerConsumerQueue<T>::try_pop() {
    lock_guard<mutex> lock(mutex_);
    if (queue_.empty()) {
        return nullopt;
    }
    T t_value = std::move(queue_.front());
    queue_.pop();
    return t_value;
}

template<typename T>
optional<T> ProducerConsumerQueue<T>::wait_and_pop() {
    unique_lock<mutex> lock(mutex_);
    condition_variable_.wait(lock, [this]() { return !queue_.empty(); });
    if (!queue_.empty()) {
        T t_value = std::move(queue_.front());
        queue_.pop();
        return t_value;
    }
    return nullopt;
}


#endif //ADAPTABLE_UPLOADER_PRODUCERCONSUMERQUEUE_H
