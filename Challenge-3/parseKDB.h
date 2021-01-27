#ifndef PARSEKDB_H
#define PARSEKDB_H

#include <iostream>
#include <string>
#include <stdint.h>
#include <vector>

#include "lfsr.h"

using namespace std;

/***********************/
/****** Constants ******/
const long NUM_MAGIC_BYTES = 6;
const long ENTRY_SIZE = 20;
const long BLOCK_SIZE = 6;
const int32_t LIST_TERMINATOR = 0xFFFFFFFF;
const int MAX_ENTRY_NAME = 16;
const unsigned int DECRYPT_KEY = 0x4F574154;
const int BYTE = 8;

/***********************/
/******* Structs *******/
struct Block {
	int16_t size;
	int32_t dataPos;
};

struct Entry {
	string name;
	unsigned char* data;
	int32_t size;
};

/***********************/
/*** Helper Functions **/
// readLittleEndian(): Reads little endian formatted data from a buffer and converts it to big endian.
// Templated for integer types, undefined behavior for non-integer types.
// Params: 	unsigned char*; buffer containing desired data
// 			int32_t; offset position of desired data within buffer
// Return:	class type (intended to be integer type); big endian conversion of desired data
template<class T>
T readLittleEndian(const unsigned char* buffer, const int32_t startPos)
{
	T bigEndianValue = 0;
	for(int i = 0; i < (int)sizeof(T); i++)
	{
		bigEndianValue += ((T)buffer[startPos + i]) << (i * BYTE);
	}
	
	return bigEndianValue;	
}

// checkForListEnd(): Checks a position of a buffer for a list terminator (checks for list ending).
// Params:	unsigned char*; buffer being checked
// 			int32_t; offset position to check for list terminator
// Return:	bool; flag for if list terminator is present. True if present, false if not found.
bool checkForListEnd(const unsigned char* buffer, const int32_t startPos)
{
	int32_t nextBufferValue = 0x0;
	for(int32_t i = (sizeof(int32_t) - 1); i >= 0; i--)
	{
		nextBufferValue += ((int32_t)buffer[startPos + i]) << (BYTE * (sizeof(int32_t) - i));
	}
	
	bool isEnd = (nextBufferValue == LIST_TERMINATOR);
	return isEnd;
}

/***********************/
/******* Parsing *******/
vector<Entry> parseKDB(const unsigned char* kdbBuffer, const int32_t bufferLen)
{
	// Read entry list position
	int32_t entryListPos = readLittleEndian<int32_t>(kdbBuffer, NUM_MAGIC_BYTES);
	
	// Read entry list
	vector<Entry> entryList;
	int32_t entryIndex = entryListPos;
	while(checkForListEnd(kdbBuffer, entryIndex) == false)
	{
		// Read entry info
		string entryName = (char*)&kdbBuffer[entryIndex];
		int32_t blockListPos = readLittleEndian<int32_t>(kdbBuffer, (entryIndex + MAX_ENTRY_NAME));
		
		// Read block list
		vector<Block> blockList;
		int32_t blockIndex = blockListPos;
		int32_t totalDataSize = 0;
		while(checkForListEnd(kdbBuffer, blockIndex) == false)
		{
			Block newBlock;
			newBlock.size = readLittleEndian<int16_t>(kdbBuffer, blockIndex);
			newBlock.dataPos = readLittleEndian<int32_t>(kdbBuffer, (blockIndex + sizeof(int16_t)));
			blockList.push_back(newBlock);

			totalDataSize += newBlock.size;
			blockIndex += BLOCK_SIZE;
		}
	
		// Read data from blocks into single buffer
		unsigned char* data = new unsigned char[totalDataSize];
		int32_t numBlocks = (int32_t)blockList.size();
		int32_t readCount = 0;
		for(int32_t i = 0; i < numBlocks; i++)
		{
			Block readBlock = blockList.at(i);
			for(int32_t k = 0; k < readBlock.size; k++)
			{
				data[readCount] = kdbBuffer[readBlock.dataPos + k];
				readCount++;
			}
		}

		// Decrypt data
		data = Crypt(data, totalDataSize, DECRYPT_KEY);
	
		// Store data
		Entry newEntry;
		newEntry.name = entryName;
		newEntry.data = data;
		newEntry.size = totalDataSize;
		entryList.push_back(newEntry);

		// Move to next entry in list
		data = NULL;
		entryIndex += ENTRY_SIZE;
	}		
	
	return entryList;
}

#endif
