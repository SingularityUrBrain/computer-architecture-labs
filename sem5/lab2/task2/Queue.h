#ifndef QUEUE_H
#define QUEUE_H

template <typename T>
class Queue
{
public:
    virtual void push(T val) = 0;
    virtual bool pop(T &val) = 0;
};

#endif