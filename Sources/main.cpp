
#include "hardware.h"
#include "mma845x.h"
#include "ioexpander.h"

using namespace USBDM;

IoExpander io;


int main() {

	io.setAllPinsAsOutput();
	io.setPinAsInput(0);

	int delay = 0;

	for (;;) {

		if (io.pinRead(0)) delay = 50;
		else delay = 0;

		for (int i = 1; i < 8; i++) {
			io.pinToggle(i);
			waitMS(delay);
		}

		waitMS(500);

	}

	return 0;

}

