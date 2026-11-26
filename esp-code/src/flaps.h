#include <Arduino.h>
#include <DRV8825.h>

class flaps {
	private:
		DRV8825 motor;

	public:
		void init(int dir, int step) { motor.begin(dir, step); }

		bool active() { return false; }

		bool activate(bool open = false) { return true; }
};