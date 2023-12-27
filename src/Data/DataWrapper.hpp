//
// Created by 86137 on 2023/12/23.
//

#ifndef ADAPTABLE_UPLOADER_DATAWRAPPER_HPP
#define ADAPTABLE_UPLOADER_DATAWRAPPER_HPP

#include "ThreadSafe/ProducerConsumerOptional.h"
#include "ThreadSafe/ProducerConsumerQueue.h"

struct DataWrapper {
    ProducerConsumerOptional<bool> should_close;
    ProducerConsumerQueue<string> recv_queue;
    ProducerConsumerQueue<string> send_queue;
};


#endif //ADAPTABLE_UPLOADER_DATAWRAPPER_HPP
