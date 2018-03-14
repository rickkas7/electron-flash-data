#include "Particle.h"
#include "electron-flash-file.h"

FlashFile ff;

const unsigned long PRINT_INTERVAL_MS = 10000;
unsigned long lastPrint = 0;
char buf[64];

static const size_t BYTES_PER_LINE = 16;

void setup() {
	Serial.begin(9600);

}

void loop() {
	if (millis() - lastPrint >= PRINT_INTERVAL_MS) {
		lastPrint = millis();

		FlashFileInfo ffi;
		int res;

		for(size_t index = 0; ; index++) {
			res = ff.getFileInfo(index, &ffi);
			if (res) {
				Serial.printlnf("index=%d res=%d", index, res);
				break;
			}

			Serial.printlnf("=======================================================================================");
			Serial.printlnf("index=%d name=%s length=%d", index, ffi.name, ffi.length);

			for(size_t offset = 0; offset < ffi.length; offset += BYTES_PER_LINE) {
				size_t count = ffi.length - offset;
				if (count > BYTES_PER_LINE) {
					count = BYTES_PER_LINE;
				}
				ff.readFileData(&ffi, offset, buf, count);

				Serial.printf("%06x: ", offset);
				for(size_t ii = 0; ii < BYTES_PER_LINE; ii++) {
					if (ii < count) {
						Serial.printf("%02x ", buf[ii]);
					}
					else {
						Serial.printf("   ");
					}
				}

				Serial.printf("  ");
				for(size_t ii = 0; ii < BYTES_PER_LINE; ii++) {
					if (ii < count && buf[ii] >= ' ' && buf[ii] < 127) {
						Serial.printf("%c", buf[ii]);
					}
					else {
						Serial.printf(" ");
					}
				}
				Serial.println("");

			}
		}
	}
}
