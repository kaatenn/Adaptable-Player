//
// Created by 86137 on 2023/12/23.
//

#ifndef ADAPTABLE_UPLOADER_DATAWRAPPER_HPP
#define ADAPTABLE_UPLOADER_DATAWRAPPER_HPP

#include "ThreadSafe/ProducerConsumerOptional.h"

struct DataWrapper {
    ProducerConsumerOptional<bool> should_close;
};


#endif //ADAPTABLE_UPLOADER_DATAWRAPPER_HPP
