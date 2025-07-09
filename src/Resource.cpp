#include <stdexcept>

#include "CAppContainer.h"
#include "App.h"
#include "Resource.h"
#include "Canvas.h"

Resource::Resource() {
	//printf("Resource::init\n");
	std::memset(this, 0, sizeof(Resource));

    this->ioBuffer = new uint8_t[Resource::IO_SIZE];
    std::memset(this->ioBuffer, 0, Resource::IO_SIZE);
}

Resource::~Resource() {
}

bool Resource::startup() {
	//printf("Resource::startup\n");

	return false;
}

void Resource::readByteArray(InputStream* IS, uint8_t* dest, int off, int size) {
	IS->read(dest, off, size);
}

void Resource::readUByteArray(InputStream* IS, short* dest, int off, int size) {
    while (size > 0) {
        int n2 = (Resource::IO_SIZE > size) ? size : Resource::IO_SIZE;
        size -= n2;
        this->read(IS, n2);
        while (--n2 >= 0) {
            dest[off++] = this->shiftUByte();
        }
    }
}

void Resource::readCoordArray(InputStream* IS, short* dest, int off, int size) {
    while (size > 0) {
        int n2 = (Resource::IO_SIZE > size) ? size : Resource::IO_SIZE;
        size -= n2;
        this->read(IS, n2);
        while (--n2 >= 0) {
            dest[off++] = this->shiftCoord();
        }
    }
}

void Resource::readShortArray(InputStream* IS, short* dest, int off, int size) {
    while (size > 0) {
        int n2 = (Resource::IO_SIZE > size) ? size : Resource::IO_SIZE;
        size -= n2;
        this->read(IS, n2 * sizeof(short));
        while (--n2 >= 0) {
            dest[off++] = this->shiftShort();
        }
    }
}

void Resource::readUShortArray(InputStream* IS, int* dest, int off, int size) {
    while (size > 0) {
        int n2 = (Resource::IO_SIZE > size) ? size : Resource::IO_SIZE;
        size -= n2;
        this->read(IS, n2 * sizeof(uint16_t));
        while (--n2 >= 0) {
            dest[off++] = this->shiftUShort();
        }
    }
}

void Resource::readIntArray(InputStream* IS, int* dest, int off, int size) {
    while (size > 0) {
        int n2 = (Resource::IO_SIZE > size) ? size : Resource::IO_SIZE;
        size -= n2;
        this->read(IS, n2 * sizeof(int));
        while (--n2 >= 0) {
            dest[off++] = this->shiftInt();
        }
    }
}

void Resource::readMarker(InputStream* IS, int i) {
    this->read(IS, sizeof(uint32_t));
}

void Resource::readMarker(InputStream* IS) {
    this->readMarker(IS, 0xCAFEBABE);
}

void Resource::writeMarker(OutputStream* OS, int i) {
    this->ioBuffer[0] = (uint8_t)(i & 0xFF);
    this->ioBuffer[1] = (uint8_t)(i >> 8 & 0xFF);
    this->ioBuffer[2] = (uint8_t)(i >> 16 & 0xFF);
    this->ioBuffer[3] = (uint8_t)(i >> 24 & 0xFF);
    OS->write(this->ioBuffer, 0, sizeof(uint32_t));
}

void Resource::writeMarker(OutputStream* OS) {
    this->writeMarker(OS, 0xCAFEBABE);
}

void Resource::read(InputStream* IS, int i) {
    this->cursor = 0;
    IS->read(this->ioBuffer, 0, i);
}

void Resource::bufSkip(InputStream* IS, int offset, bool updateLB) {
    IS->offsetCursor(offset);
    if (updateLB) {
        CAppContainer::getInstance()->app->canvas->updateLoadingBar(false);
    }
}

uint8_t Resource::byteAt(int i) {
    return this->ioBuffer[i];
}

uint8_t Resource::shiftByte() {
    return this->ioBuffer[this->cursor++];
}

short Resource::UByteAt(int i) {
    return (short)(this->ioBuffer[i] & 0xFF);
}

short Resource::shiftUByte() {
    return (short)(this->ioBuffer[this->cursor++] & 0xFF);
}

short Resource::shortAt(int i) {
    return (short)((this->ioBuffer[i + 0] & 0xFF) + (this->ioBuffer[i + 1] << 8 & 0xFF00));
}

short Resource::shiftShort() {
    short dat = (short)((this->ioBuffer[this->cursor + 0] & 0xFF) + (this->ioBuffer[this->cursor + 1] << 8 & 0xFF00));
    this->cursor += sizeof(short);
    return dat;
}

int Resource::shiftUShort() {
    int dat = (int)((this->ioBuffer[this->cursor + 0] & 0xFF) + (this->ioBuffer[this->cursor + 1] << 8 & 0xFF00));
    this->cursor += sizeof(uint16_t);
    return dat;
}

int Resource::shiftInt() {
    int dat = (int)(this->ioBuffer[this->cursor + 3] << 24 & 0xFF000000) | (this->ioBuffer[this->cursor + 2] << 16 & 0xFF0000) | (this->ioBuffer[this->cursor + 1] << 8 & 0xFF00) | (this->ioBuffer[this->cursor + 0] & 0xFF);
    this->cursor += sizeof(int);
    return dat;
}

short Resource::shiftCoord() {
    return (short)((this->ioBuffer[this->cursor++] & 0xFF) * 8);
}

int* Resource::readFileIndex(InputStream *IS) {
    this->read(IS, sizeof(int));
    int shiftInt = this->shiftInt();
    int* array = new int[shiftInt];
    this->read(IS, shiftInt * sizeof(int));
    for (int i = 0; i < shiftInt; ++i) {
        array[i] = this->shiftInt();
    }
    return array;
}

int* Resource::loadFileIndex(char* fileName) {
    Applet* app = CAppContainer::getInstance()->app;
    InputStream IS;
    //printf("loadFileIndex::init\n");

    if (IS.loadFile(fileName, InputStream::LOADTYPE_RESOURCE) == false) {
        app->Error("getResource(%s) failed\n", fileName);
    }

    this->read(&IS, sizeof(int16_t));

    short count = this->shiftShort();
    int* array = new int[count * 3];
    int n3 = (5 * (Resource::IO_SIZE / 5));
    short n4 = 0;
    do {
        int n5 = (count - n4) * 5;
        int n6 = (n5 > n3) ? n3 : n5;
        this->read(&IS, n6);
        for (int i = 0; i < n6; i += 5) {
            uint8_t _shiftByte = this->shiftByte();
            int _shiftInt = this->shiftInt();
            if (_shiftInt != 0) {
                array[(n4 * 3) - 1] = _shiftInt - array[(n4 * 3) - 2];
            }
            if (_shiftByte != 0xff) {
                array[(n4 * 3) + 0] = _shiftByte;
                array[(n4 * 3) + 1] = _shiftInt;
                ++n4;
            }
        }
    } while (n4 != count);
    this->read(&IS, 5);
    this->shiftByte();
    array[(n4 * 3) - 1] = this->shiftInt() - array[(n4 * 3) - 2];
    IS.close();
    IS.~InputStream();
    return array;
}

void Resource::initTableLoading() {
    Applet* app = CAppContainer::getInstance()->app;
    InputStream IS;

    if (IS.loadFile(Resources::RES_TABLES_BIN_GZ, InputStream::LOADTYPE_RESOURCE) == false) {
        app->Error("getResource(%s) failed\n", Resources::RES_TABLES_BIN_GZ);
    }

    this->read(&IS, 80);
    for (int i = 0; i < 20; ++i) {
        this->tableOffsets[i] = this->shiftInt();
    }
    IS.close();
    IS.~InputStream();
}

void Resource::beginTableLoading() {
    Applet* app = CAppContainer::getInstance()->app;

    if (this->prevIS.loadFile(Resources::RES_TABLES_BIN_GZ, InputStream::LOADTYPE_RESOURCE) == false) {
        app->Error("getResource(%s) failed\n", Resources::RES_TABLES_BIN_GZ);
    }

    this->bufSkip(&this->prevIS, 80, false);
    this->prevOffset = 0;
}

void Resource::seekTable(int index) {
    Applet* app = CAppContainer::getInstance()->app;

    int offset = 0;
    if (index > 0) {
        offset = this->tableOffsets[index - 1];
    }
    if (offset < this->prevOffset) {
        app->Error("seekTable seeking backwards\n");
    }
    this->bufSkip(&this->prevIS, offset - this->prevOffset, false);
    this->prevOffset = offset;
}

void Resource::finishTableLoading() {
    this->prevIS.close();
}

int Resource::getNumTableBytes(int index) {
    int offset = this->tableOffsets[index] - 4;
    if (index == 0) {
        return offset;
    }
    return offset - this->tableOffsets[index - 1];
}

int Resource::getNumTableShorts(int index) {
    int offset = this->tableOffsets[index] - 4;
    if (index == 0) {
        return offset / sizeof(short);
    }
    return (offset - this->tableOffsets[index - 1]) / sizeof(short);
}

int Resource::getNumTableInts(int index) {
    int offset = this->tableOffsets[index] - 4;
    if (index == 0) {
        return offset / sizeof(int);
    }
    return (offset - this->tableOffsets[index - 1]) / sizeof(int);
}

void Resource::loadByteTable(int8_t* array, int index) {
    this->seekTable(index);
    this->read(&this->prevIS, sizeof(int));
    int size = shiftInt();
    int i = 0;
    int n3 = 0;
    while (i < size) {
        int n4 = (i + Resource::IO_SIZE > size) ? (size - i) : Resource::IO_SIZE;
        this->read(&this->prevIS, n4);
        for (int j = 0; j < n4; j++) {
            array[n3++] = this->shiftByte();
        }
        i += Resource::IO_SIZE;
    }
    this->prevOffset += sizeof(int) + size;
}

int Resource::loadShortTable(short* array, int index) {
    this->seekTable(index);
    this->read(&this->prevIS, sizeof(int));
    int size = shiftInt() * sizeof(short);
    int i = 0;
    int n3 = 0;
    while (i < size) {
        int n4 = (i + Resource::IO_SIZE > size) ? (size - i) : Resource::IO_SIZE;
        this->read(&this->prevIS, n4);
        for (int j = 0; j < n4; j += sizeof(short)) {
            array[n3++] = this->shiftShort();
        }
        i += Resource::IO_SIZE;
    }
    this->prevOffset += sizeof(int) + size;
    return n3; // [GEC]: new
}

void Resource::loadIntTable(int* array, int index) {
    this->seekTable(index);
    this->read(&this->prevIS, sizeof(int));
    int size = shiftInt() * sizeof(int);
    int i = 0;
    int n3 = 0;
    while (i < size) {
        int n4 = (i + Resource::IO_SIZE > size) ? (size - i) : Resource::IO_SIZE;
        this->read(&this->prevIS, n4);
        for (int j = 0; j < n4; j += sizeof(int)) {
            array[n3++] = this->shiftInt();
        }
        i += Resource::IO_SIZE;
    }
    this->prevOffset += sizeof(int) + size;
}

void Resource::loadUByteTable(uint8_t* array, int index) {
    this->seekTable(index);
    this->read(&this->prevIS, sizeof(int));
    int size = shiftInt();
    int i = 0;
    int n3 = 0;
    while (i < size) {
        int n4 = (i + Resource::IO_SIZE > size) ? (size - i) : Resource::IO_SIZE;
        this->read(&this->prevIS, n4);
        for (int j = 0; j < n4; j++) {
            array[n3++] = (uint8_t)this->shiftUByte();
        }
        i += Resource::IO_SIZE;
    }
    this->prevOffset += sizeof(int) + size;
}

void Resource::loadUShortTable(uint16_t* array, int index) {
    this->seekTable(index);
    this->read(&this->prevIS, sizeof(int));
    int size = shiftInt() * sizeof(short);
    int i = 0;
    int n3 = 0;
    while (i < size) {
        int n4 = (i + Resource::IO_SIZE > size) ? (size - i) : Resource::IO_SIZE;
        this->read(&this->prevIS, n4);
        for (int j = 0; j < n4; j += sizeof(short)) {
            array[n3++] = (uint16_t)this->shiftUShort();
        }
        i += Resource::IO_SIZE;
    }
    this->prevOffset += sizeof(int) + size;
}
