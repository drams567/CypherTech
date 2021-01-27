#ifndef LSFR_H
#define LSFR_H

#include <stdlib.h>
#include <cstring>

/*******************/
/**** Constants ****/
const unsigned int LSFR_FEEDBACK_VALUE = 0x87654321; 	// Feedback value for lsfr alg
const unsigned int LSFR_NUM_STEPS = 8;					// Steps needed to generate new key

/*******************/
/***** Helper ******/
// getNewKey(): Generates a new key from an initial value using the lsfr algorithm.
// Params: unsigned int; initial value to used to generate a new key. May be a previously generated key.
// Return: unsigned int; new key value.
unsigned int getNewKey(const unsigned int oldKey)
{
 	// Step through lsfr alg
 	unsigned int newKey = oldKey;
    for(unsigned int i = 0; i < LSFR_NUM_STEPS; i++) {
        // Check last bit
		if((newKey & 0x1) == 0x0) // if last bit is 0
        {
            newKey = newKey >> 1;
        }
        else
        {
            newKey = (newKey >> 1) ^ LSFR_FEEDBACK_VALUE;
        }
    }

    return newKey;
}

/*****************/
/***** Crypt *****/
// Crypt(): Encrypt (and decrypt) data using the lsfr algorithm.
// Notes: Encrypted data replaces original data passed in.
// Params: unsigned char*; data to be encrypted.
// 		   int; length of the data.
// 		   unsigned int; initial value/key used for the encryption.
// Return: unsigned char*; same pointer passed in, now with the data encrypted.
unsigned char* Crypt(unsigned char* data, int dataLength, unsigned int initialValue) 
{
	// Place data into buffer
	unsigned char* dataBuffer = (unsigned char*)calloc(dataLength, sizeof(unsigned char));
	memcpy(dataBuffer, data, dataLength);

	// Encrypt
	unsigned int key = getNewKey(initialValue);
	for(int i = 0; i < dataLength; i++)
	{
		dataBuffer[i] = dataBuffer[i] ^ key;
		key = getNewKey(key);
	}

	// Replace data
	memcpy(data, dataBuffer, dataLength);

	// Clean buffer
	free(dataBuffer);
	dataBuffer = NULL;

	return data;
}


#endif
