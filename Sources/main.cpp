
#include "hardware.h"
#include "mma845x.h"
#include "ioexpander.h"

using namespace USBDM;

IoExpander io;


int main() {

	io.setAllPinsAsOutput();

	for (;;) {

		for (int i = 0; i < 8; i++) {
			io.pinToggle(i);
			waitMS(500);
		}

	}

	return 0;

}

