#include<Queue.h>

#ifndef QUEUE_DEF
#define QUEUE_DEF

template <typename DataType>
Queue<DataType>::Queue(Config * config) {
    cout << "4" << endl;
    this->id = config->get_name();
    this->size = (uint8_t)config->parameters["Size"];
    // cout << "Queue: " << this->id << "\t" << "Size: " << this->size << endl;
    this->registers.resize(this->size);
    // cout << "Queue: " << this->id << "\t" << "Size: " << this->size << endl;
    this->readPointer = 0;
    this->writePointer = 0;
    this->count = 0;
    this->empty = true;
    this->full = false;
    cout << "Read Pointer: " << this->readPointer << endl;
    cout << "Write Pointer: " << this->writePointer << endl;
    cout << "Count: " << this->count << endl;
    cout << "Full: " << this->full << endl;
    cout << "Empty: " << this->empty << endl;
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
unsigned int Queue<DataType>::getCount() {
    return this->count;
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
void Queue<DataType>::print() {
    cout << "========================================================" << endl;
    cout << "Queue: " << this->id << "\t" << this->size << endl;
    cout << "Idx\t Value" << endl;
    cout << "--------------------------------------------------------" << endl;
    int i = 0;
    for (DataType reg: this->registers) {
        cout << i << '\t' << reg << endl;
        i++;
    }
    cout << "Read Pointer: " << this->readPointer << endl;
    cout << "Write Pointer: " << this->writePointer << endl;
    cout << "Count: " << this->count << endl;
    cout << "Full: " << this->full << endl;
    cout << "Empty: " << this->empty << endl;
}

#endif