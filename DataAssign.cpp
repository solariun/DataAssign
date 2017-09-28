/*
# =======================================================
# Data Assign 
#
# Programmer: Gustavo Campos
#
# Date: Sep, 22 2017
#=========================================================
*/

using namespace std;

#include "Util.h"
#include <string>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include <stdio.h>
#include <unistd.h>



class CircularDataBuffer
{	
private:
	uint8_t* 	charBuffer;
	uint32_t 	nBufferLen;
	uint32_t	nOffset;
	uint32_t    nBufferStrLen;

protected:
	
	void appendChar (const uint8_t nChar)
	{
		uint32_t nWhere = 0;
		
        nWhere = (nOffset + nBufferStrLen) % nBufferLen;
        
        nBufferStrLen++;
        
    
        if (nBufferStrLen > nBufferLen)
        {
            nOffset = (++nOffset) > nBufferLen ? 0 : nOffset;
            nBufferStrLen = nBufferLen;
        }
        
        
        charBuffer [nWhere] = nChar;
        
        printf ("Offset: [%d], BLEN: [%d], LEN: [%d] nWhere: [%d]  -  |", nOffset, nBufferStrLen, nBufferLen, nWhere);
        
        for (int a=0; a < nBufferLen; a++) printf (" %c%c |",  charBuffer [a] < 32 ? '-' : (uint8_t)charBuffer [a], a == nOffset ? '*' : ' ');
        printf ("\n");
	}

public:
    
	CircularDataBuffer (uint32_t nSize)
	{
		charBuffer = new uint8_t (nSize);
		nOffset = 0; 
		nBufferLen = nSize;
		nBufferStrLen = 0;
	}
	
    void appendString(const uint8_t* pszData, uint32_t nStrLen)
    {
        Verify (pszData != NULL, "Error, pszReturn is null.", );
        Verify (nStrLen > 0, "Error, nRetLen must be gratter than 0.", );
        
        uint32_t nReadOffset = 0;
        
        
        /*
        if (nStrLen > nBufferLen)
        {
            nReadOffset = nStrLen - nBufferLen - 1;
        }
        */
        
        for (;nReadOffset < nStrLen; nReadOffset++)
        {
            appendChar (pszData [nReadOffset]);
        }
    }
	
    
    char* getCStrBuffer(char* pszReturn, uint32_t nRetLen)
    {
        Verify (pszReturn != NULL, "Error, pszReturn is null.", NULL);
        Verify (nRetLen > 0, "Error, nRetLen must be gratter than 0.", NULL);
        
        int nLen = (nRetLen-1) < nBufferStrLen ? nRetLen-1 : nBufferStrLen;
        
        uint32_t nLOffset = nLOffset;
        uint32_t nWhere   = this->nOffset - 1;
        
        for (nLOffset = 0; nLOffset < nLen; nLOffset++) { nWhere = ++nWhere >= nBufferLen ? 0 : nWhere; printf ("nWhere: [%d]\n", nWhere); pszReturn[nLOffset] = charBuffer [nWhere]; }
        pszReturn[nLOffset] = '\0';
        
        return pszReturn;
    }
};




#define SIGN_FIRST_START 0x3A334F3BL

const char *getUserName()
{
  uid_t uid = geteuid();
  struct passwd *pw = getpwuid(uid);
  if (pw)
  {
    return pw->pw_name;
  }

  return "";
}

typedef union _datablk
{
	uint64_t _data;
	uint16_t Values16b [4];
	uint32_t Values32b [2];
	uint8_t	 Values8b  [8];
} DataBlk64;
	

#define SIGN_HARD_SIGNATURE "ERICSSON"

	
uint64_t appendToAssign (DataBlk64* i64Deck, const char* pszData, uint32_t nDataSize)
{
    Verify (pszData != NULL, "Error, Data is NULL.", 0);
    Verify (i64Deck != NULL, "Error, i64Deck in NULL.", 0);
	
    i64Deck [0]._data = TUtiL_crc64Binary ((const uint8_t*)pszData, nDataSize, i64Deck [0]._data);
    i64Deck [1]._data = TUtiL_crc64Binary ((const uint8_t*)pszData, nDataSize, i64Deck [1]._data);
    i64Deck [2]._data = TUtiL_crc64Base2Binary ((const uint8_t*)pszData, nDataSize, i64Deck [2]._data );
    
    //TRACE ("\n\tCRC1: [%llX]\n\tCRC2: [%llX]\n\tCRC3: [%llX]\n\t LEN: [%d]\n\n", i64Deck [0]._data, i64Deck [1]._data, i64Deck [2]._data, nDataSize); 
	
	return i64Deck [0]._data; 
}


    
DataBlk64* getNewDataBlk64()
{
	DataBlk64* pDataBlk64 = (DataBlk64*) malloc (sizeof (DataBlk64) * 3);
	
	pDataBlk64[0]._data =  120323LLU;
	pDataBlk64[1]._data =  0x0A04A1LLU;
	pDataBlk64[2]._data =  0xF458934AFLLU;

	appendToAssign (pDataBlk64, SIGN_HARD_SIGNATURE, strlen (SIGN_HARD_SIGNATURE));
	
	return pDataBlk64;
}



uint64_t getAssign (DataBlk64* i64Deck, char* pszReturn, uint32_t nRetLen)
{
	Verify (i64Deck != NULL, "Error, i64Deck in NULL.", 0);
    Verify (pszReturn != NULL, "Error, Return is NULL.", 0);
	Verify (nRetLen != 0, "WARNING, nothing to do return is 0 length.", 0);
	
    snprintf (pszReturn, nRetLen, "%08X-%04X-%04X%04X-%04X-%08X", //36posicoes.
              i64Deck [1].Values32b [0],
              i64Deck [2].Values16b [0], 
              i64Deck [2].Values16b [1], 
              i64Deck [2].Values16b [2], 
              i64Deck [2].Values16b [3],
              i64Deck [1].Values32b [1]);
    
    return i64Deck [0]._data; 
}



/*
.20170925.145105.FAECAC08.029BE8C6B21A2B7D.E12C7495-72DA-DF6C59A8-722F-C8A44FFF
*/
bool appendFirstSign(char* pszData, const int nDataLen)
{
	Verify (pszData !=NULL, "Error, pszData is NULL.", false);
	Verify (strlen (pszData) < (nDataLen-10), "Error, pszData size unconformity.", false);
	
	int nLen = strlen (pszData);
	
	//printf ("Data: [%s]\n", pszData);
	snprintf (pszData + nLen, sizeof (pszData) - nLen, ".%016llX.",TUtiL_crc64Binary ((const uint8_t*) pszData, (unsigned int) strlen(pszData), (unsigned long) SIGN_FIRST_START));
	
	return true;
}

int main(int nArgs, char** ppszArgs)
{
	Verify (isatty (fileno(stdin)) == true, "ERROR, PIPE is not supported.", -10);
	
	timespec ts;
    // clock_gettime(CLOCK_MONOTONIC, &ts); // Works on FreeBSD
    clock_gettime(CLOCK_REALTIME, &ts); // Works on Linux	
 
    srand (time (NULL) * (ts.tv_sec & ts.tv_nsec));
   
	if (nArgs < 2)
	{
		printf ("%s - Use <Assign <File_name> <comment_char>|Check <File_name>\n");
		exit (1);
	}
	
		
	
	if (strncmp ("Assign", ppszArgs [1], strlen ("Assign")) == 0)
	{
		
		Verify (nArgs != 3, "Error, not enough arguments. Assin <File_name> <comment_char>", -1);
		
		int nRand = rand ();
		long int nSALT = (long) ((double)((double)nRand / RAND_MAX) * 0xFFFFFFFF);
		
		time_t rawtime;
		struct tm *info;
		char strNow[25], strInfo[100];

		time( &rawtime );

		info = localtime( &rawtime );

		strftime(strNow,sizeof(strNow)-1,"%Y%m%d.%H%M%S", info);
		
		snprintf (strInfo, sizeof(strInfo)-1, "%cSIGNED.%s.%s.%08lX", ppszArgs [3][0], getUserName (), strNow, nSALT);

		appendFirstSign (strInfo, sizeof (strInfo));
		
		//printf ("%s (%d)\n\n", strInfo, strlen(strInfo));
		
		FILE* pFile;
		
		if((pFile = fopen (ppszArgs [2], "r")) == NULL)
		{
			printf ("Error, opening file [%s] - %s\n", ppszArgs [2], strerror (errno));
			exit (-2);
		}
		
		char strBlock [1025] = "";
		uint nReadSoFar = 0;
		
		DataBlk64* i64Deck = getNewDataBlk64 ();
		
		while (feof (pFile) == false)
		{
			nReadSoFar = fread(strBlock, 1, 1024, pFile);
			strBlock[nReadSoFar] = '\0';
			
			fwrite (strBlock, 1, nReadSoFar, stdout);
			
			appendToAssign (i64Deck, strBlock, nReadSoFar);
		}
		
		appendToAssign (i64Deck, strInfo, strlen(strInfo));
		fwrite (strInfo, 1, strlen(strInfo), stdout);
		
		char pszSignature [50];
		
		getAssign (i64Deck, pszSignature, sizeof (pszSignature));
		
		printf ("%s\n", pszSignature);
		
		fclose (pFile);
	}
	else if (strncmp ("Check", ppszArgs [1], strlen ("Assign")) == 0)
	{
		Verify (nArgs != 2, "Error, not enough arguments. Check <File_name>", -1);
		
		FILE* pFile;
		
		if((pFile = fopen (ppszArgs [2], "r")) == NULL)
		{
			printf ("Error, opening file [%s] - %s\n", ppszArgs [2], strerror (errno));
			exit (-2);
		}
		
		char strBlock [1025] = "";
		long nReadSoFar = 0;
		
		DataBlk64* i64Deck = getNewDataBlk64 ();
		
		fseek (pFile, 0, SEEK_END);
		long nFileSize = ftell (pFile);
		fseek (pFile, 0, SEEK_SET);
		
		
		Verify (nFileSize > 80, "Error, fine may not being assigned.", -1);
		
		//printf ("Size: [%d]\n", nFileSize);
		
		//size of UUID
		nFileSize = nFileSize - 37;
		
		//printf ("Size: [%d]\n", nFileSize);
		
		long nDiff = 0;

        char pszChar[2];
        
        CircularDataBuffer cBuffer (37);
		for (int a=0; a < (nFileSize); a += nReadSoFar)
		{
			nDiff = (nFileSize - a);
			
			nReadSoFar = fread((uint8_t*) strBlock, 1, nDiff > 1024 ? 1024 : nDiff, pFile);
			strBlock[nReadSoFar] = '\0';

			//fwrite (strBlock, 1, nReadSoFar, stdout);
			
			//printf ("  ...... [%ld]", a); 
			//printf ("[%ld]", nReadSoFar); 
			//printf ("[%ld]\n", nFileSize);
			
			
			appendToAssign (i64Deck, strBlock, nReadSoFar);
		}
		
		char pszFileSignature [37];
		
		
		pszFileSignature [fread (pszFileSignature, 1, 36, pFile)] = '\0';
		
		char pszSignature [50];
		getAssign (i64Deck, pszSignature, sizeof (pszSignature));
		
		printf ("Remote signature: [%s]\n", pszFileSignature);
		printf ("Calc.  signature: [%s]\n", pszSignature);
		
		if (strncmp (pszSignature, pszFileSignature, 36) == 0)
		{
			printf ("Signature OK\n"); 
			exit (0);
		}
		else
		{
			printf ("Signature ERROR\n"); 
			exit (1);			
		}
		
		//UTil_PrintDataToDebug ((uint8_t*) pszFileSignature, sizeof (pszFileSignature));
		
		
		printf ("%s", pszSignature);
		
		fclose (pFile);
	}
	
	return 0;
}
