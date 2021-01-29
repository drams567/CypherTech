#include <iostream>
#include <fstream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "parseKDB.h"
#include "md5.h"		// MD5 hash library, Provided by: http://www.zedwood.com/article/cpp-md5-function

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
private:
	int32_t size;
	int32_t offset;
	string hash;
	string outPath;
	unsigned char* data;

public:
	Jpeg()
	{
		size = 0;
		offset = 0;
		hash = " ";
		outPath = " ";
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
	
	void print()
	{
		cout << getOffset() << " " << getSize() << " " << getHash() << " " << getOutPath();
	}
	
	void setOutPath(string newOutPath) { outPath = newOutPath; }
	void setHash(string newHash) { hash = newHash; }
	void setData(unsigned char* newData) { data = newData; }
	void setOffset(int32_t newOffset) { offset = newOffset; }
	void setSize(int32_t newSize) { size = newSize; }
	
	unsigned char* getData() { return data; }
	int32_t getOffset() { return offset; }
	int32_t getSize() { return size; }
	string getHash() { return hash; }
	string getOutPath() { return outPath; }
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
void outputJpegs(vector<Jpeg> &jpegList, const string inputFileName)
{
	string outputDir = inputFileName + "_Repaired";
	
	#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
		// Create directory (Windows), Reference: https://docs.microsoft.com/en-us/windows/win32/fileio/retrieving-and-changing-file-attributes
		#include <windows.h>
		CreateDirectory("./" + outputDir);
	#elif __linux__ || __unix__
		// Create directory (Linux/Unix), References: https://linux.die.net/man/3/mkdir, https://pubs.opengroup.org/onlinepubs/7908799/xsh/sysstat.h.html
		mkdir(outputDir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
	#endif
	
	for(vector<Jpeg>::iterator jpegIt = jpegList.begin(); jpegIt != jpegList.end(); jpegIt++)
	{
		string outPath = outputDir + "/" + to_string(jpegIt->getOffset()) + ".jpeg";
		jpegIt->setOutPath(outPath);
		
		ofstream jpegStream;
		jpegStream.open(outPath, ostream::binary | ostream::trunc);
		jpegStream.write((char*)jpegIt->getData(), jpegIt->getSize());
		jpegStream.close();
		
		jpegIt->print();
		cout << endl;
	}
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
			newJpeg.setOffset(i);
			i += numMagicBytes;
			while(checkMatch(&inputBuffer[i], JPEG_TERMINATOR, JPEG_TERMINATOR_SIZE) == false)
			{
				i++;
			}
			newJpeg.setSize((i + JPEG_TERMINATOR_SIZE) - newJpeg.getOffset());
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
		data = new unsigned char[jpegIt->getSize()];
		memcpy(data, &inputBuffer[jpegIt->getOffset()], jpegIt->getSize());
		
		// Repair obfuscated starting bytes
		memcpy(data, JPEG_START, JPEG_START_SIZE);

		// Save data
		jpegIt->setData(data);
		data = NULL;
	}
	
	// Clean input file buffer
	delete [] inputBuffer;
	inputBuffer = NULL;

	// Calculate hash
	for(vector<Jpeg>::iterator jpegIt = jpegList.begin(); jpegIt != jpegList.end(); jpegIt++)
	{
		string data((char*)jpegIt->getData(), jpegIt->getSize());
		string hash = md5(data); // courtesy of md5 library: http://www.zedwood.com/article/cpp-md5-function
		jpegIt->setHash(hash);
	}

	// Output jpegs
	outputJpegs(jpegList, inputFileName);

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
