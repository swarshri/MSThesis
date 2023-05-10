#include<iostream>
#include<vector>
#include<bitset>
#include<string>

#include<Config.h>

#ifndef QUEUE_H
#define QUEUE_H

template <typename DataType>
class Queue {
    public:
        Queue(SysConfig *, uint32_t = 1);

        void push(DataType);
        DataType pop();
        DataType next();

        unsigned int getCount();
        unsigned int getEmptyCount();
        bool isEmpty();
        bool isFull();
        void resize(int);

        void show(ostream&);

    private:
        vector<DataType> registers;
        uint32_t readPointer;
        uint32_t writePointer;
        uint32_t size;
        uint32_t count;
        string id;
        bool empty;
        bool full;
};

#include <../src/Queue.cpp>
#endif