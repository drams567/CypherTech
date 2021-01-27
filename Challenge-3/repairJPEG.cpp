#include <iostream>
#include <fstream>
#include <filesystem>

#include "parseKDB.h"

using namespace std;

/***********************/
/****** Constants ******/
const unsigned char JPEG_START[] = {0xFF, 0xD8, 0xFF};
const unsigned char JPEG_TERMINATOR[] = {0xFF, 0xD9};
const int32_t JPEG_START_SIZE = 3;
const int32_t JPEG_TERMINATOR_SIZE = 2;

/***********************/
/******* Classes *******/
class Jpeg {
public:
	int32_t size;
	int32_t offset;
	long hash;
	string outFilePath;
	unsigned char* data;

	Jpeg()
	{
		size = 0;
		offset = 0;
		hash = 0;
		outFilePath = " ";
		data = NULL;
	}
	~Jpeg()
	{
		if(data != NULL)
		{
			delete [] data;
			data = NULL;
		}
	}
};

/***********************/
/******* Parsing *******/
bool checkMatch(const unsigned char* source, const unsigned char* match, const int32_t size)
{
	for(int i = 0; i < size; i++)
	{
		if(source[i] != match[i])
		{
			return false;
		}
	}

	return true;
}

/***********************/
/***** Procedures ******/
void outputJpegs(const vector<Jpeg> &jpegList, const string inputFileName)
{
	
}

/***********************/
/********* Main ********/
int main(int argc, char* argv[])
{
	// Open kdb file stream
	string kdbFileName = argv[1];
	ifstream kdbStream;
	kdbStream.open(kdbFileName, ifstream::binary | ifstream::in); // read as binary, Reference: http://www.cplusplus.com/reference/istream/istream/read/
	
	// Get file length, Reference: http://www.cplusplus.com/reference/istream/istream/read/
	kdbStream.seekg(0, kdbStream.end);
	int kdbStreamLen = kdbStream.tellg();
	kdbStream.seekg(0, kdbStream.beg);

	// Put kdbFile into buffer
	unsigned char* kdbBuffer = new unsigned char[kdbStreamLen];
	kdbStream.read((char*)kdbBuffer, kdbStreamLen);
	kdbStream.close();

	// Parse buffer for entries
	vector<Entry> entryList = parseKDB(kdbBuffer, kdbStreamLen);

	// Clean buffer
	delete [] kdbBuffer;
	kdbBuffer = NULL;

	// Find magic bytes
	unsigned char* magicBytes = NULL;
	int32_t numMagicBytes = 0;
	int numEntries = (int)entryList.size();
	for(int i = 0; i < numEntries; i++)
	{
		if(entryList.at(i).name == "MAGIC")
		{
			magicBytes = entryList.at(i).data;
			numMagicBytes = entryList.at(i).size;
		}
	}

	// Check for missing entry
	if(magicBytes == NULL)
	{
		cerr << "No magic entry found in KDB file. Quitting..." << endl;
		exit(0);
	}

	// Put input file into buffer
	string inputFileName = argv[2];
	ifstream inputStream;
	inputStream.open(inputFileName, ifstream::binary | ifstream::in); // read as binary, Reference: http://www.cplusplus.com/reference/istream/istream/read/
	inputStream.seekg(0, inputStream.end);
	int inputStreamLen = inputStream.tellg();
	inputStream.seekg(0, inputStream.beg);
	unsigned char* inputBuffer = new unsigned char[inputStreamLen];
	inputStream.read((char*)inputBuffer, inputStreamLen);

	// First pass, identify jpegs
	vector<Jpeg> jpegList;
	int32_t i = 0;
	while(i < inputStreamLen)
	{
		if(checkMatch(&inputBuffer[i], magicBytes, numMagicBytes) == true)
		{
			Jpeg newJpeg;
			newJpeg.offset = i;
			i += numMagicBytes;
			while(checkMatch(&inputBuffer[i], JPEG_TERMINATOR, JPEG_TERMINATOR_SIZE) == false)
			{
				i++;
			}
			newJpeg.size = (i + JPEG_TERMINATOR_SIZE) - newJpeg.offset;
			jpegList.push_back(newJpeg);

			i += JPEG_TERMINATOR_SIZE - 1;
		}
		else
		{
			i++;
		}
	}

	// Read and repair jpegs
	unsigned char* data = NULL;
	for(vector<Jpeg>::iterator jpegIt = jpegList.begin(); jpegIt != jpegList.end(); jpegIt++)
	{
		// Read jpeg data
		data = new unsigned char[jpegIt->size];
		memcpy(data, &inputBuffer[jpegIt->offset], jpegIt->size);
		
		// Repair obfuscated starting bytes
		memcpy(data, JPEG_START, JPEG_START_SIZE);

		jpegIt->data = data;
		data = NULL;
	}
	
	// Clean input file buffer
	delete [] inputBuffer;
	inputBuffer = NULL;

	// Output jpegs
	outputJpegs(jpegList, inputFileName);

	// Clean jpegs	
	for(vector<Jpeg>::iterator jpegIt = jpegList.begin(); jpegIt != jpegList.end(); jpegIt++)
	{
		if(jpegIt->data != NULL)
		{
			delete [] jpegIt->data;
			jpegIt->data = NULL;
		}
	}

	// Clean entries
	for(i = 0; i < (int32_t)entryList.size(); i++)
	{
		if(entryList.at(i).data != NULL)
		{
			delete [] entryList.at(i).data;
			entryList.at(i).data = NULL;
		}
	}

	return 0;
}
