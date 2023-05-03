#include<Queue.h>

#ifndef QUEUE_DEF
#define QUEUE_DEF

template <typename DataType>
Queue<DataType>::Queue(SysConfig * config, uint8_t overridden_size) {
    this->id = config->get_name();
    uint8_t config_size = (uint8_t)config->parameters["Size"];
    if (overridden_size > config_size)
        this->size = overridden_size;
    else
        this->size = (uint8_t)config->parameters["Size"];
    this->registers.resize(this->size);
    this->readPointer = 0;
    this->writePointer = 0;
    this->count = 0;
    this->empty = true;
    this->full = false;
}

template <typename DataType>
void Queue<DataType>::push(DataType value) {
    this->registers[writePointer] = value;
    this->count++;
    this->empty = false;
    if (this->writePointer == this->size - 1)
        this->writePointer = 0;
    else
        this->writePointer++;
    if (this->readPointer == this->writePointer)
        this->full = true;
}

template <typename DataType>
DataType Queue<DataType>::pop() {
    DataType toReturn = this->registers[readPointer];
    this->count--;
    this->full = false;
    if (this->readPointer == this->size - 1)
        this->readPointer = 0;
    else
        this->readPointer++;
    if (this->readPointer == this->writePointer)
        this->empty = true;
    return toReturn;
}

template <typename DataType>
DataType Queue<DataType>::next() {
    return this->registers[readPointer];
}

template <typename DataType>
unsigned int Queue<DataType>::getCount() {
    return this->count;
}

template <typename DataType>
unsigned int Queue<DataType>::getEmptyCount() {
    return this->size - this->count;
}

template <typename DataType>
void Queue<DataType>::resize(int newsize) {
    if (newsize > this->size) {
        this->size = newsize;
        this->registers.resize(this->size);
    }
}

template <typename DataType>
bool Queue<DataType>::isEmpty() {
    return this->empty;
}

template <typename DataType>
bool Queue<DataType>::isFull() {
    return this->full;
}

template <typename DataType>
void Queue<DataType>::show(ostream& op) {
    op << "........................................" << endl;
    op << "Queue: " << this->id << "\t" << this->size << endl;
    op << "Idx\t| Value" << endl;
    int i = 0;
    for (DataType reg: this->registers) {
        op << i << "\t| " << reg << endl;
        i++;
    }
    op << "Read Pointer: " << this->readPointer << endl;
    op << "Write Pointer: " << this->writePointer << endl;
    op << "Count: " << this->count << endl;
    op << "Full: " << this->full << endl;
    op << "Empty: " << this->empty << endl;
    op << ".........................................." << endl;
}

#endif