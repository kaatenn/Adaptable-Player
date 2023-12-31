//
// Created by 86137 on 2023/12/23.
//

#ifndef ADAPTABLE_UPLOADER_DATAWRAPPER_HPP
#define ADAPTABLE_UPLOADER_DATAWRAPPER_HPP

#include "api/ProducerConsumerOptional.h"
#include "api/ProducerConsumerQueue.h"

struct DataWrapper {
    ProducerConsumerOptional<bool> should_close;
    ProducerConsumerQueue<string> recv_queue; // We do not need to device the queue into two parts, the ui doesn't care.
    ProducerConsumerQueue<string> kcp_send_queue;
    ProducerConsumerQueue<string> tcp_send_queue;
};


#endif //ADAPTABLE_UPLOADER_DATAWRAPPER_HPP
