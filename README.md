# electron-flash-data

*Sample code for using the factory backup sector in the Electron (and E-Series) to store data.*

There's an extra 128K sector of flash memory (the "factory backup sector") that can be used to store additional data for your application. This sample allows you to store file-like data, things such as:

- Certificates
- Sound files
- Image files

It's possible to store code there, but it's tricky and this example won't cover that. Also note that this is intended to be used for data that doesn't change and can be set using USB (DFU).

The idea is that you create an filesystem image file on your computer containing all of the files you want to be able to access. This image file is then downloaded to the device by USB in DFU mode.

Note that this is only example code, to show you how it could be done. It's not intended to be a finished product.

Also note that this cannot be used on the Photon/P1. The reason is that even though the Photon memory map shows a factory backup sector, when you do a system firmware update OTA, this sector is overwritten, thus making it far less useful than the Electron. 

I intended to write a version that's able to update the sector from an HTTP server, however I get an error trying to write that sector. I think the system firmware prevents overwriting the backup firmware slot, which is not an unreasonable behavior.

## Using the imagetool

In order to create a single file containing the virtual file system, use the imagetool. It's written in node.js and is in the imagetool directory.

First, install dependencies:

```
npm install
```

The run it like this:

```
node imagetool.js -v -o file.bin directory_with_files
```

Basically, put all your files you want in your image in a directory, and point the image tool it. It will create a file (file.bin in the example above) with all of the files in the directory, along with various header information and CRCs for validation.

Note: the directory must be flat, with no subdirectories. Subdirectories are not supported!


## Uploading using USB

For the Electron and E-series, you can upload the file you just created by doing:

```
dfu-util -d 2b04:d00a -a 0 -s 0x80a0000 -D file.bin
```


## Using the 1-printimage example

From the top-level electron-flash-data directory you can compile and flash the code like this using the Particle CLI compiler:

```
particle compile electron examples/1-printimage/ --saveTo firmware.bin
particle flash --usb firmware.bin 
```

This example just prints out in hex and ASCII all of the files on the file system image that you uploaded.
