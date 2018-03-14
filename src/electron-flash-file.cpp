#include "Particle.h"

#include "electron-flash-file.h"

FlashFile::FlashFile(size_t addrStart) : addrStart(addrStart), numFiles(0) {
}

FlashFile::~FlashFile() {

}

int FlashFile::validate() {

	FlashFileHeader hdr;
	memcpy(&hdr, (const void *)addrStart, sizeof(hdr));

	if (hdr.magic != FLASHFILE_HEADER_MAGIC || hdr.version > FLASHFILE_HEADER_VERSION) {
		return FLASHFILE_INVALID_FORMAT;
	}
	numFiles = hdr.numFiles;

	return FLASHFILE_NO_ERROR;
}

int FlashFile::getFileInfo(size_t index, FlashFileInfo *info) {
	int res = validate();
	if (res == FLASHFILE_NO_ERROR) {
		if (index < numFiles) {
			size_t addr = addrStart + sizeof(FlashFileHeader) + index * sizeof(FlashFileInfo);

			memcpy(info, (const void *)addr, sizeof(FlashFileInfo));
		}
		else {
			res = FLASHFILE_FILE_NOT_FOUND;
		}
	}
	return res;
}

int FlashFile::getFileByName(const char *name, FlashFileInfo *info) {
	for(size_t ii = 0; ii < numFiles; ii++) {
		FlashFileInfo infoTemp;
		getFileInfo(ii, &infoTemp);
		if (strcmp(name, infoTemp.name) == 0) {
			if (info != NULL) {
				memcpy(info, (const void *)&infoTemp, sizeof(FlashFileInfo));
			}
			return FLASHFILE_NO_ERROR;
		}
	}
	return FLASHFILE_FILE_NOT_FOUND;
}

void *FlashFile::getFileAddress(FlashFileInfo *info) {
	return (void *)(addrStart + info->addr);
}


int FlashFile::readFileData(FlashFileInfo *info, size_t offset, char *buf, int bufLen) {

	if (offset >= info->length) {
		return FLASHFILE_END_OF_FILE;
	}

	if ((offset + bufLen) > info->length) {
		// Don't allow read past the actual end of the file
		bufLen = info->length - offset;
	}

	memcpy(buf, (void *)(addrStart + info->addr + offset), bufLen);

	return FLASHFILE_NO_ERROR;
}

