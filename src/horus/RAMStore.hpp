#pragma once
#include "Types.hpp"
#include <map>
#include <array>

class RAMStore {
	std::vector<block> store;
	size_t size;
        size_t emptyNodes;

public:
	RAMStore(size_t num, size_t size);
	~RAMStore();

	block Read(int pos);
	void Write(int pos, block b);

	size_t GetBlockCount();
	size_t GetBlockSize();        
	bool WasSerialised();
        void ReduceEmptyNumbers();
        size_t GetEmptySize();
};
