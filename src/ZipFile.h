#ifndef __ZIPFILE_H__
#define __ZIPFILE_H__

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <stdint.h>
#include <zlib.h>

#define ZIP_LOCAL_FILE_SIG 0x04034b50
#define ZIP_CENTRAL_DIRECTORY_SIG 0x02014b50
#define ZIP_END_OF_CENTRAL_DIRECTORY_SIG 0x06054b50
#define ZIP_ENCRYPTED_FLAG 0x1

typedef struct zip_entry_s
{
	char* name;
	int offset;
	int csize, usize;
}zip_entry_t;

class ZipFile
{
private:
	FILE* file;
	int entry_count;
	zip_entry_t* entry;
	int page_count;
	int* page;
public:

	// Constructor
	ZipFile();
	// Destructor
	~ZipFile();

	void findAndReadZipDir(int startoffset);
	void openZipFile(const char* name);
	void closeZipFile();
	uint8_t* readZipFileEntry(const char* name, int* sizep);
};

#endif