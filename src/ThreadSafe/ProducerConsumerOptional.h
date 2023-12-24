//
// Created by 86137 on 2023/12/23.
//

#ifndef ADAPTABLE_UPLOADER_PRODUCERCONSUMEROPTIONAL_H
#define ADAPTABLE_UPLOADER_PRODUCERCONSUMEROPTIONAL_H

#include "queue"
#include "mutex"
#include "condition_variable"
#include "string"

using std::mutex, std::condition_variable, std::string, std::lock_guard, std::optional, std::nullopt,
        std::unique_lock;

template<typename T>
class ProducerConsumerOptional {
public:
    void set(T &&t_value);

    optional<T> get_and_reset();
private:
    optional<T> value_;
    mutex mutex_;
};

template<typename T>
void ProducerConsumerOptional<T>::set(T &&t_value) {
    lock_guard<mutex> lock(mutex_);
    value_ = std::move(t_value);
}

template<typename T>
optional<T> ProducerConsumerOptional<T>::get_and_reset() {
    lock_guard<mutex> lock(mutex_);
    if (value_.has_value()) {
        auto tmp = std::move(value_);
        value_.reset();
        return tmp;
    } else {
        return nullopt;
    }
}

#endif //ADAPTABLE_UPLOADER_PRODUCERCONSUMEROPTIONAL_H
