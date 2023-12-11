#include <stdexcept>

#include "JavaStream.h"
#include "CAppContainer.h"
#include "App.h"
#include "ZipFile.h"

// ------------------
// InputStream Class
// ------------------

InputStream::InputStream() {
	//printf("InputStream::init\n");
	this->data = nullptr;
	this->field_0x4 = 0;
	this->cursor = 0;
	this->field_0x0 = 0;
	this->file = nullptr;
	this->field_0x28 = 0;
}

InputStream::~InputStream() {
	if (this->file) {
		std::fclose(this->file);
		this->file = nullptr;
	}

	if (this->data) {
		std::free(this->data);
		this->data = nullptr;
	}
}

bool InputStream::loadResource(const char* fileName)
{
	return loadFile(fileName, LT_RESOURCE);
}

bool InputStream::loadFile(const char* fileName, int loadType) {
	char namePath[2048];
	this->cursor = 0;

	if (loadType == LT_RESOURCE) {
		strncpy(namePath, "Payload/Doom2rpg.app/Packages/", sizeof(namePath));
		strncat(namePath, fileName, sizeof(namePath));
		//printf("namePath %s\n", namePath);

		this->data = CAppContainer::getInstance()->zipFile->readZipFileEntry(namePath, &this->fileSize);
		if (this->data) {
			return true;
		}
	}
	else if (loadType == LT_SOUND_RESOURCE) { // [GEC]
		strncpy(namePath, "Payload/Doom2rpg.app/Packages/sounds2/", sizeof(namePath));
		strncat(namePath, fileName, sizeof(namePath));
		//printf("namePath %s\n", namePath);

		this->data = CAppContainer::getInstance()->zipFile->readZipFileEntry(namePath, &this->fileSize);
		if (this->data) {
			return true;
		}
	}
	else if (loadType == LT_FILE) {
		std::strcpy(namePath, dir);
		std::strcat(namePath, "/");
		std::strcat(namePath, fileName);

		this->file = std::fopen(namePath, "rb");

		if (this->file != nullptr) {
			std::fseek(this->file, 0, SEEK_END);
			this->fileSize = ftell(this->file);
			std::fseek(this->file, 0, SEEK_SET);
			this->data = (uint8_t*)std::malloc(this->fileSize);
			std::fread(this->data, sizeof(uint8_t), this->fileSize, this->file);

			if (std::ferror(this->file)) {
				CAppContainer::getInstance()->app->Error( "Error Reading File!\n");
				return false;
			}

			std::fclose(this->file);
			return true;
		}
		else {
			//CAppContainer::getInstance()->app->Error("Error Openeing File: %s\n", namePath);
		}
	}
	else {
		CAppContainer::getInstance()->app->Error("File does not exist\n");
	}

	return false;
}

void InputStream::close() {
	if (this->file) {
		std::fclose(this->file);
		this->file = nullptr;
	}
}

int InputStream::readInt()
{
	int ch1 = this->data[this->cursor + 3];
	int ch2 = this->data[this->cursor + 2];
	int ch3 = this->data[this->cursor + 1];
	int ch4 = this->data[this->cursor + 0];
	this->cursor += sizeof(int32_t);
	return (int) ((ch1 << 24 & 0xFF000000) + (ch2 << 16 & 0xFF0000) + (ch3 << 8 & 0xFF00) + (ch4 << 0 & 0xFF));
}

int InputStream::readShort()
{
	int ch1 = this->data[this->cursor + 1];
	int ch2 = this->data[this->cursor + 0];
	this->cursor += sizeof(int16_t);
	return (int)((ch1 << 8 & 0xFF00) + (ch2 << 0 & 0xFF));
}

bool InputStream::readBoolean()
{
	uint8_t ch1 = this->data[this->cursor + 0];
	this->cursor += sizeof(int8_t);
	return (bool)((ch1 != 0) ? true : false);
}

uint8_t InputStream::readByte()
{
	uint8_t ch1 = this->data[this->cursor + 0];
	this->cursor += sizeof(int8_t);
	return ch1;
}

uint8_t InputStream::readUnsignedByte()
{
	uint8_t ch1 = this->data[this->cursor + 0];
	this->cursor += sizeof(int8_t);
	return ch1;
}

int InputStream::readSignedByte()
{
	uint8_t ch1 = this->data[this->cursor + 0];
	this->cursor += sizeof(int8_t);
	return (int)((char)ch1);
}

void InputStream::read(uint8_t* dest, int off, int size) {
	for (int i = 0; i < size; i++) {
		if (this->cursor >= this->fileSize) {
			break;
		}
		dest[off + i] = this->data[this->cursor + 0];
		this->cursor++;
	}
}


// -------------------
// OutputStream Class
// -------------------

OutputStream::OutputStream() {
	//printf("OutputStream::init\n");
	this->buffer = nullptr;
	this->writeBuff = nullptr;
	this->file = nullptr;
	this->App = CAppContainer::getInstance()->app;;
	this->field_0x24_ = -1;
	this->written = 0;
	this->flushCount = 0;
	this->isOpen = false;
	this->noWrite = false;
}

OutputStream::~OutputStream() {
	this->close();

	if (this->writeBuff) {
		std::free(this->writeBuff);
		this->writeBuff = nullptr;
	}

	if (this->buffer != nullptr) {
		std::free(this->buffer);
		this->buffer = nullptr;
	}
}

#include <sys/stat.h>

int OutputStream::openFile(const char* fileName, int openMode) {
	char namePath[2060];

	struct stat sb;
	if (stat(dir, &sb)) {
		char command[64];
		std::strcpy(command, "mkdir ");
		std::strcat(command, "\"");
		std::strcat(command, dir);
		std::strcat(command, "\"");
		//printf("command %s\n", command);
		std::system(command);
	}

	std::strcpy(namePath, dir);
	std::strcat(namePath, "/");
	std::strcat(namePath, fileName);

	//printf("output file: %s\n", namePath);
	this->buffer = (uint8_t*)std::malloc(512);
	std::memset(this->buffer, 0, 512);

	if (this->file != nullptr) {
		this->isOpen = true;
		return 0;
	}

	this->isOpen = false;
	if (this->noWrite != false) {
		return 1;
	}

	switch (openMode)
	{
	case 1:
		this->file = std::fopen(namePath, "wb");
		break;
	case 2:
		this->file = std::fopen(namePath, "rb");
		break;
	case 3:
		this->file = std::fopen(namePath, "w+b");
		break;
	default:
		return 0;
	}

	if (this->file != nullptr) {
		std::fseek(this->file, 0, SEEK_END);
		this->fileSize = std::ftell(this->file);
		std::fseek(this->file, 0, SEEK_SET);
		this->writeBuff = (uint8_t*)std::malloc(this->fileSize);
		return 1;
	}

	return 0;
}

void OutputStream::close()
{
	this->flush();
	if (this->file != nullptr) {
		std::fclose(this->file);
		this->file = nullptr;
	}

	if (this->buffer != nullptr) {
		std::free(this->buffer);
		this->buffer = nullptr;
	}
}

int OutputStream::flush()
{
	if (this->written == 0) {
		return 1;
	}

	this->flushCount += this->written;

	if (!this->noWrite) {
		if (std::fwrite(this->buffer, sizeof(uint8_t), this->written, this->file) <= 0) {
			return 0;
		}
	}

	this->written = 0;
	return 1;
}

void OutputStream::writeInt(int i)
{
	if ((256 - this->written) < sizeof(uint32_t)) {
		if (this->flush() == 0) {
			return;
		}
	}

	this->buffer[this->written++] = (uint8_t)(i & 0xFF);
	this->buffer[this->written++] = (uint8_t)(i >> 8 & 0xFF);
	this->buffer[this->written++] = (uint8_t)(i >> 16 & 0xFF);
	this->buffer[this->written++] = (uint8_t)(i >> 24 & 0xFF);
}

void OutputStream::writeShort(int16_t i)
{
	if ((256U - this->written) < sizeof(uint16_t)) {
		if (this->flush() == 0) {
			return;
		}
	}

	this->buffer[this->written++] = (uint8_t)(i & 0xFF);
	this->buffer[this->written++] = (uint8_t)(i >> 8 & 0xFF);
}

void OutputStream::writeByte(uint8_t i)
{
	if ((256U - this->written) < sizeof(uint8_t)) {
		if (this->flush() == 0) {
			return;
		}
	}

	this->buffer[this->written++] = (uint8_t)(i & 0xFF);
}

void OutputStream::writeBoolean(bool b)
{
	if ((256U - this->written) < sizeof(uint8_t)) {
		if (this->flush() == 0) {
			return;
		}
	}

	this->buffer[this->written++] = (uint8_t)(b);
}

void OutputStream::write(uint8_t* buff, int off, int size)
{
	int _written = this->written;
	if ((uint32_t)(_written + size) >= 256) {
		int count = 256 - _written;
		std::memcpy(&this->buffer[_written], &buff[off], count);
		this->written += count;
		size -= count;
		off += count;
		if (this->flush() == 0) {
			return;
		}
	}

	std::memcpy(&this->buffer[_written], &buff[off], size);
	this->written += size;
}
