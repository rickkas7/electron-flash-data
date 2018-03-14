// Install dependencies:
// npm install
//
// Run this like:
// node imagetool.js -v -o file.bin directory_with_files
//
// Flash like:
// dfu-util -d 2b04:d00a -a 0 -s 0x80a0000 -D file.bin

var fs = require('fs');
var path = require('path');

// https://www.npmjs.com/package/buffer-crc32
var crc32 = require('buffer-crc32');

// https://www.npmjs.com/package/mime-types
var mime = require('mime-types');

// yargs argument parser (successor to optimist)
// https://www.npmjs.com/package/yargs
var argv = require('yargs')
	.usage('Usage: $0 -o <outputfile> [options] <inputfile> ...')
	.describe('o', 'Output to binary image data file')
	.describe('v', 'Verbose output')
	.demand(1)
	.boolean('v')
	.help('h')
	.alias('h', 'help')
	.argv;

var files = [];

// argv._ is an array of files/folders to add to the image

processInputList();

buildImage();
//-o <outputfile>



function processInputList() {
	for(var ii = 0; ii < argv._.length; ii++) {
		var pathname = argv._[ii];
		processFileOrDir(pathname);
	}
}

function buildImage() {
	if (argv.v) {
		console.log(files.length + ' files to add to image');
	}

	// Image Header
	/*
	const uint32_t FLASHFILE_HEADER_MAGIC = 0xe0917763;
	const uint32_t FLASHFILE_HEADER_VERSION = 1;
	typedef struct { // 48 bytes
		uint32_t	magic;			// offset = 0x00
		uint32_t	version;		// offset = 0x04
		uint32_t	numFiles;		// offset = 0x08
		uint32_t	totalSize;		// offset = 0x0c
		uint32_t	crc;			// offset = 0x10
		uint32_t	modDate;		// offset = 0x14
		uint32_t	reserved[6];	// offset = 0x18
	} FlashFileHeader;
	*/
	
	// We'll set the modification date to this when done
	var modDateMs = Date.now();
	
	// Calculate total size of image
	var headerSize = 48;
	var infoSize = 80;
	
	var totalLength = headerSize + files.length * infoSize;
	console.log("headers and info records before adding file data length=" + totalLength);
	for(var ii = 0; ii < files.length; ii++) {
		files[ii].addr = totalLength;
		totalLength += files[ii].size;
	}
	console.log("totalLength=" + totalLength + " files.length=" + files.length);
	
	var image = new Buffer(totalLength);
	image.fill(0);
	image.writeUInt32LE(0xe0917763, 0x00); // magic
	image.writeUInt32LE(1, 0x04); // version
	image.writeUInt32LE(files.length, 0x08); // numFiles
	image.writeUInt32LE(totalLength, 0x0c); // totalLength
	image.writeUInt32LE(modDateMs / 1000, 0x14); // modDate (in seconds)
	// CRC written below
	
	var curOffset = headerSize;
	
	for(var ii = 0; ii < files.length; ii++) {
		/*
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
		*/
		var fileInfo = new Buffer(infoSize);
		fileInfo.fill(0);

		var actualNameLen = fileInfo.write(files[ii].name, 0);
		if (actualNameLen > 31) {
			// This check is necessary in case f.name has Unicode character that expand to more than 31 bytes after UTF-8 encoding
			console.log('file ' + f.name + ' has too long of a name after encoding');
			throw 'name too long';						
		}
		fileInfo.writeUInt32LE(files[ii].mtime, 0x20);
		fileInfo.writeUInt32LE(files[ii].size, 0x24);
		
		fileInfo.writeUInt32LE(files[ii].addr, 0x28);
		
		// Add in the content-type at 0x30
		var actualContentTypeLen = fileInfo.write(files[ii].contentType, 0x30);
		if (actualContentTypeLen > 31) {
			console.log('file ' + f.name + ' has a content type ' + f.contentType + ' that is too long');
			throw 'name too long';						
		}
		
		fileInfo.copy(image, curOffset);
		curOffset += infoSize;
	}
	
	// Copy the file data
	for(var ii = 0; ii < files.length; ii++) {
		if (files[ii].size > 0) {
			files[ii].data.copy(image, curOffset);
			curOffset += files[ii].size;
		}
	}	
	
	// Write the final CRC
	var crcBuf = crc32(image);
	image.writeUInt8(crcBuf.readUInt8(3), 0x10);
	image.writeUInt8(crcBuf.readUInt8(2), 0x11);
	image.writeUInt8(crcBuf.readUInt8(1), 0x12);
	image.writeUInt8(crcBuf.readUInt8(0), 0x13);
	
	if (argv.o) {
		// Write all of the data out to the image file
		var writer = fs.createWriteStream(argv.o);
		
		// Image 
		writer.write(image);
		
		fs.utimesSync(argv.o, modDateMs, modDateMs);
	}
}

	
function processFileOrDir(pathname) {
	var stats = fs.statSync(pathname);
	if (stats.isFile()) {
		var f = {};
		
		f.pathname = pathname;
		f.name = path.basename(pathname);
		if (f.name.length > 31) {
			console.log('file ' + f.name + ' has too long of a name');
			throw 'name too long';			
		}
		f.mtime = Math.floor(stats.mtime.getTime() / 1000);
		f.size = stats.size;
		f.data = fs.readFileSync(pathname);
		f.contentType = mime.lookup(f.name) || 'application/octet-stream';

		if (f.name == 'LICENSE' || f.name == 'README') {
			f.contentType = 'text/plain';
		}
		
		files.push(f);

		console.log("file", f);
	}
	else
	if (stats.isDirectory()) {
		processDir(pathname);
	}
	else {
		console.log('file ' + pathname + ' is not a file or directory');
		throw 'not a file or directory';
	}
}

function processDir(pathname) {
	var items = fs.readdirSync(pathname);
	for(var ii = 0; ii < items.length; ii++) {
		processFileOrDir(path.join(pathname, items[ii]));
	}
} 
