#include <ArduinoNvs.h>

class Settings {
	private:
		ArduinoNvs nvs;

	public:
		Settings() { nvs.begin("settings"); }
};