#ifndef __FLASHFILE_H
#define __FLASHFILE_H

#include "Particle.h"

#if PLATFORM_ID == PLATFORM_PHOTON_PRODUCTION || PLATFORM_ID == PLATFORM_P1
// Photon, P1
# define DEFAULT_START_ADDR 0x80e0000
#elif PLATFORM_ID == PLATFORM_ELECTRON_PRODUCTION
// Electron, E Series
# define DEFAULT_START_ADDR 0x80a0000
#else
# error("unsupported platform")
#endif


enum {
	FLASHFILE_NO_ERROR = 0,
	FLASHFILE_NO_CHIP,
	FLASHFILE_INVALID_FORMAT,
	FLASHFILE_FILE_NOT_FOUND,
	FLASHFILE_END_OF_FILE
};

const size_t FLASHFILE_MAX_NAME_LEN = 31;
const size_t FLASHFILE_MAX_CONTENT_TYPE_LEN = 31;
typedef struct { // 80 bytes (0x50)
	char 		name[FLASHFILE_MAX_NAME_LEN + 1];	// offset = 0x00
	uint32_t 	modDate;		// offset = 0x20
	uint32_t	length;			// offset = 0x24
	uint32_t	addr;			// offset = 0x28
	uint32_t	reserved;		// offset = 0x2c
	char 		contentType[FLASHFILE_MAX_CONTENT_TYPE_LEN + 1];	// offset = 0x30
} FlashFileInfo;

const uint32_t FLASHFILE_HEADER_MAGIC = 0xe0917763;
const uint32_t FLASHFILE_HEADER_VERSION = 1;

typedef struct { // 48 bytes (0x30)
	uint32_t	magic;			// offset = 0x00
	uint32_t	version;		// offset = 0x04
	uint32_t	numFiles;		// offset = 0x08
	uint32_t	totalSize;		// offset = 0x0c
	uint32_t	crc;			// offset = 0x10
	uint32_t	modDate;		// offset = 0x14
	uint32_t	reserved[6];	// offset = 0x18
} FlashFileHeader;

class FlashFile {
public:
	FlashFile(size_t addrStart = DEFAULT_START_ADDR);
	virtual ~FlashFile();

	/**
	 * Validate the FlashFileHeader
	 */
	int validate();

	/**
	 * Get information about file by its index
	 *
	 * You can use this to iterate the files; keep calling until FLASHFILE_FILE_NOT_FOUND is returned.
	 *
	 * index Specifies which file to get. Zero-based index.
	 */
	int getFileInfo(size_t index, FlashFileInfo *info);

	/**
	 * Get information about a file from its filename
	 *
	 * Note: Filenames are case-sensitive! There are no directories, but you can have a filename that
	 * contains a / to simulate directories. However the maximum filename length is still 31 bytes
	 * including all components.
	 */
	int getFileByName(const char *name, FlashFileInfo *info);

	/**
	 * Get the address of the file in memory
	 */
	void *getFileAddress(FlashFileInfo *info);

	/**
	 * Copy data from a file
	 *
	 * There is no concept of open or closed files as they're all memory mapped.
	 */
	int readFileData(FlashFileInfo *info, size_t offset, char *buf, int bufLen);


private:
	size_t addrStart;
	size_t numFiles;
};

#endif /* __FLASHFILE_H */
