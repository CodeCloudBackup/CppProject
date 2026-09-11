#ifndef RESULT_TYPE_H
#define RESULT_TYPE_H

#include "any_type.h"
#include "semaphore_type.h"

class ResultType {
public:
    ResultType() = default;
    ~ResultType() = default;
private:
    AnyType result_;
    SemaphoreType semaphore_;
};


#endif
