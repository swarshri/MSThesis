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
        Queue(Config *);

        void push(DataType);
        DataType pop();

        unsigned int getCount();
        bool isEmpty();
        bool isFull();

        void print();

    private:
        vector<DataType> registers;
        int readPointer;
        int writePointer;
        int size;
        int count;
        string id;
        bool empty;
        bool full;
};

#include <../src/Queue.cpp>
#endif