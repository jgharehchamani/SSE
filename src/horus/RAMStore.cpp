#include "RAMStore.hpp"
#include <iostream>
#include "ORAM.hpp"
using namespace std;

RAMStore::RAMStore(size_t count, size_t size)
: store(count), size(size),emptyNodes(count)
{}

RAMStore::~RAMStore()
{}

block RAMStore::Read(int pos)
{
	return store[pos];
}

void RAMStore::Write(int pos, block b)
{
	store[pos] = b;
}

size_t RAMStore::GetBlockCount()
{
	return store.size();
}

size_t RAMStore::GetBlockSize()
{
	return size;
}

bool RAMStore::WasSerialised()
{
	return false;
}

void RAMStore::ReduceEmptyNumbers() {
    emptyNodes--;
}

size_t RAMStore::GetEmptySize() {
    return emptyNodes;
}
