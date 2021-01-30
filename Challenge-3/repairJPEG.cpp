#include <iostream>
#include <fstream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "parseKDB.h"
#include "md5.h"     // MD5 hash library, Provided by: http://www.zedwood.com/article/cpp-md5-function

using namespace std;

/***********************/
/****** Constants ******/
const unsigned char JPEG_START[] = {0xFF, 0xD8, 0xFF}; // Starting indicator bytes of a jpeg file
const unsigned char JPEG_TERMINATOR[] = {0xFF, 0xD9}; // Terminating bytes of a jpeg file
const int32_t JPEG_START_SIZE = 3;                    // Number of jpeg indicating bytes
const int32_t JPEG_TERMINATOR_SIZE = 2;               // Number of jpeg terminating bytes

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
	// Constructors and Destructor
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
	
	// Print
	void print()
	{
		cout << getOffset() << " " << getSize() << " " << getHash() << " " << getOutPath();
	}
	
	
	// Setters
	void setOutPath(string newOutPath) { outPath = newOutPath; }
	void setHash(string newHash) { hash = newHash; }
	void setOffset(int32_t newOffset) { offset = newOffset; }
	void setSize(int32_t newSize) { size = newSize; }
	void setData(unsigned char* newData, int32_t newSize) 
	{ 
		data = newData; 
		size = newSize;
	}
	
	// Getters
	unsigned char* getData() { return data; }
	int32_t getOffset() { return offset; }
	int32_t getSize() { return size; }
	string getHash() { return hash; }
	string getOutPath() { return outPath; }
};

/***********************/
/******* Utility *******/
// readFileToBuffer(): Reads a binary file into a byte array.
// Ignores passed value of buffer argument, and size argument.
// Params:  string; name of binary file
//          (OUT) unsigned char*; buffer holding binary file data
//          (OUT) int32_t; size of the buffer/binary file data
void readFileToBuffer(const string fileName, unsigned char* &buffer, int32_t &size)
{
	ifstream fileStream;
	fileStream.open(fileName, ifstream::binary | ifstream::in); // read as binary, Reference: http://www.cplusplus.com/reference/istream/istream/read/
	
	// Get file length, Reference: http://www.cplusplus.com/reference/istream/istream/read/
	fileStream.seekg(0, fileStream.end);
	size = fileStream.tellg();
	fileStream.seekg(0, fileStream.beg);

	// Read entire file into buffer
	buffer = new unsigned char[size];
	fileStream.read((char*)buffer, size);
	
	fileStream.close();
}

/***********************/
/******* Parsing *******/
// checkMatch(): compares two byte arrays for equivalent data.
// Size assumed to be greater than zero, and less than or equal to the length of the smallest byte array.
// Params:  unsigned char*; first byte array for comparison.
//          unsigned char*; second byte array for comparison.
//          size; number of bytes to compare.
// Return: bool; flag if arrays have matching data. True for equivalent, false for not equivalent.
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
void readMagicBytesFromKDB(const string kdbFileName, unsigned char* &magicBytes, int32_t &numMagicBytes)
{
	// Read kdb file into buffer
	int32_t kdbStreamLen = 0;
	unsigned char* kdbBuffer = NULL;
	readFileToBuffer(kdbFileName, kdbBuffer, kdbStreamLen);

	// Parse kdb for entries
	vector<Entry> entryList = parseKDB(kdbBuffer, kdbStreamLen);

	// Clean buffer
	delete [] kdbBuffer;
	kdbBuffer = NULL;

	// Find magic bytes
	bool found = false;
	vector<Entry>::iterator entryIt = entryList.begin();
	while(found == false && entryIt != entryList.end())
	{
		if(entryIt->name == "MAGIC")
		{
			numMagicBytes = entryIt->size;
			magicBytes = new unsigned char[numMagicBytes];
			memcpy(magicBytes, entryIt->data, numMagicBytes);
			
			found = true;
		}
		entryIt++;
	}
	
	// Clean entries
	for(vector<Entry>::iterator entryIt = entryList.begin(); entryIt != entryList.end(); entryIt++)
	{
		if(entryIt->data != NULL)
		{
			delete [] entryIt->data;
			entryIt->data = NULL;
		}
	}
}

vector<Jpeg> readJpegsFromInput(const string inputFileName, unsigned char* magicBytes, int32_t numMagicBytes)
{
	vector<Jpeg> jpegList;
	
	// Read input file to buffer
	int inputStreamLen = 0;
	unsigned char* inputBuffer = NULL;
	readFileToBuffer(inputFileName, inputBuffer, inputStreamLen);
	
	// First pass, identify jpegs
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
			int32_t size = (i + JPEG_TERMINATOR_SIZE) - newJpeg.getOffset();
			newJpeg.setSize(size);
			jpegList.push_back(newJpeg);

			i += JPEG_TERMINATOR_SIZE - 1;
		}
		else
		{
			i++;
		}
	}

	// Second pass, read and repair jpegs
	unsigned char* data = NULL;
	for(vector<Jpeg>::iterator jpegIt = jpegList.begin(); jpegIt != jpegList.end(); jpegIt++)
	{
		// Read jpeg data
		data = new unsigned char[jpegIt->getSize()];
		memcpy(data, &inputBuffer[jpegIt->getOffset()], jpegIt->getSize());
		
		// Repair obfuscated starting bytes
		memcpy(data, JPEG_START, JPEG_START_SIZE);

		// Save data
		jpegIt->setData(data, jpegIt->getSize());
		data = NULL;
	}
	
	return jpegList;
}

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
	// Parse kdb for magic bytes
	string kdbFileName = argv[1];
	unsigned char* magicBytes = NULL;
	int32_t numMagicBytes = 0;
	readMagicBytesFromKDB(kdbFileName, magicBytes, numMagicBytes);

	// Check for missing entry
	if(magicBytes == NULL || numMagicBytes == 0)
	{
		cerr << "Magic bytes not found. Quitting..." << endl;
		exit(0);
	}

	// Retrieve obfuscated jpegs
	string inputFileName = argv[2];
	vector<Jpeg> jpegList = readJpegsFromInput(inputFileName, magicBytes, numMagicBytes);

	// Calculate hash
	for(vector<Jpeg>::iterator jpegIt = jpegList.begin(); jpegIt != jpegList.end(); jpegIt++)
	{
		string data((char*)jpegIt->getData(), jpegIt->getSize());
		string hash = md5(data); // courtesy of md5 library: http://www.zedwood.com/article/cpp-md5-function
		jpegIt->setHash(hash);
	}

	// Output jpegs
	outputJpegs(jpegList, inputFileName);

	// Clean (magic bytes)
	delete [] magicBytes;
	magicBytes = NULL;

	return 0;
}
