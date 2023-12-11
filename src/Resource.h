#ifndef __RESOURCE_H__
#define __RESOURCE_H__

#include "JavaStream.h"

class InputStream;

class Resources
{
public:
    static constexpr char* RES_LOGO_BMP_GZ = "logo.bmp";
    static constexpr char* RES_LOGO2_BMP_GZ = "logo2.bmp";
    static constexpr char* RES_STRINGS_IDX_GZ = "strings.idx";
    static constexpr char* RES_TABLES_BIN_GZ = "tables.bin";
    static constexpr char* RES_STRINGS_ARRAY[] = { "strings00.bin", "strings01.bin", "strings02.bin" };
    static constexpr char* RES_ENTITIES_BIN_GZ = "entities.bin";
    static constexpr char* RES_MENUS_BIN_GZ = "menus.bin";
    static constexpr char* RES_NEWMAPPINGS_BIN_GZ = "newMappings.bin";
    static constexpr char* RES_MAP_FILE_ARRAY[] = { "map00.bin" , "map01.bin", "map02.bin", "map03.bin", "map04.bin", "map05.bin", "map06.bin", "map07.bin", "map08.bin", "map09.bin"};
    static constexpr char* RES_NEWPALETTES_BIN_GZ = "newPalettes.bin";
    static constexpr char* RES_NEWTEXEL_FILE_ARRAY[] = {
        "newTexels000.bin", "newTexels001.bin", "newTexels002.bin", "newTexels003.bin", "newTexels004.bin",
        "newTexels005.bin", "newTexels006.bin", "newTexels007.bin", "newTexels008.bin", "newTexels009.bin",
        "newTexels010.bin", "newTexels011.bin", "newTexels012.bin", "newTexels013.bin", "newTexels014.bin", 
        "newTexels015.bin", "newTexels016.bin", "newTexels017.bin", "newTexels018.bin", "newTexels019.bin",
        "newTexels020.bin", "newTexels021.bin", "newTexels022.bin", "newTexels023.bin", "newTexels024.bin",
        "newTexels025.bin", "newTexels026.bin", "newTexels027.bin", "newTexels028.bin", "newTexels029.bin",
        "newTexels030.bin", "newTexels031.bin", "newTexels032.bin", "newTexels033.bin", "newTexels034.bin",
        "newTexels035.bin", "newTexels036.bin", "newTexels037.bin", "newTexels038.bin" };
};

class Resource
{
private:

public:
    static constexpr int IO_SIZE = 20480;

    int touchMe;
    int cursor;
    uint8_t* ioBuffer;
    int tableOffsets[20];
    int prevOffset;
    InputStream prevIS;

	// Constructor
	Resource();
	// Destructor
	~Resource();

	bool startup();
    void readByteArray(InputStream* IS, uint8_t* dest, int off, int size);
    void readUByteArray(InputStream* IS, short* dest, int off, int size);
    void readCoordArray(InputStream* IS, short* dest, int off, int size);
    void readShortArray(InputStream* IS, short* dest, int off, int size);
    void readUShortArray(InputStream* IS, int* dest, int off, int size);
    void readIntArray(InputStream* IS, int* dest, int off, int size);
    void readMarker(InputStream* IS, int i);
    void readMarker(InputStream* IS);
    void writeMarker(OutputStream* OS, int i);
    void writeMarker(OutputStream* OS);
    void read(InputStream* IS, int i);
    void bufSkip(InputStream* IS, int off, bool updateLB);
    uint8_t byteAt(int i);
    uint8_t shiftByte();
    short UByteAt(int i);
    short shiftUByte();
    short shortAt(int i);
    short shiftShort();
    int shiftUShort();
    int shiftInt();
    short shiftCoord();
    int* readFileIndex(InputStream* IS);
    int* loadFileIndex(char* fileName);
    void initTableLoading();
    void beginTableLoading();
    void seekTable(int index);
    void finishTableLoading();
    int getNumTableBytes(int index);
    int getNumTableShorts(int index);
    int getNumTableInts(int index);
    void loadByteTable(int8_t* array, int index);
    int loadShortTable(short* array, int index);
    void loadIntTable(int32_t* array, int index);
    void loadUByteTable(uint8_t* array, int index);
    void loadUShortTable(uint16_t* array, int index);
};

#endif