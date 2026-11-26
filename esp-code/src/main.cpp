#include <ArduinoNvs.h>
#include <CSV_Parser.h>
#include "flaps.h"
#include "website.h"

#include "FireAPI.h"

ArduinoNvs SettingsNVS;

flaps flaps;

FireAPI fireAPI({0, 0, 0, 10, API_KEY});

String getData(String event) {
	if (event == "settings") {
		JsonDocument json;
		json["min_distance"]		   = SettingsNVS.getFloat("trigDist");
		json["house_location"]["lat"]  = SettingsNVS.getFloat("lat");
		json["house_location"]["long"] = SettingsNVS.getFloat("lon");

		String data;
		serializeJson(json, data);
		return data;
	} else if (event == "data") {
		String data;
		serializeJson(fireAPI.getData(), data);
		return data;
	} else if (event == "status") {
		return (flaps.active()) ? "closed" : "open";
	} else if (event == "logs") {
		CSV_Parser parser("ssss");
		File logs = LittleFS.open("/logs.csv", "r");

		while (logs.available()) { parser << logs.readString(); }
		logs.close();

		char** date	   = (char**)parser["date"];
		char** time	   = (char**)parser["time"];
		char** action  = (char**)parser["action"];
		char** trigger = (char**)parser["trigger"];

		JsonDocument json;
		JsonDocument entry;
		JsonArray logArray = json["actions"].to<JsonArray>();

		for (int i = 0; i < parser.getRowsCount(); i++) {
			entry["date"]	 = date[i];
			entry["time"]	 = time[i];
			entry["action"]	 = action[i];
			entry["trigger"] = trigger[i];

			logArray.add(entry);
		}

		String data;
		serializeJson(json, data);
		return data;
	} else {
		throw "Invalid event";
	}
}

int processWebRequest(String event, JsonVariant& data) {
	if (event == "settings") {
		if (!data["house_location"]["lat"].is<float>()
			|| !data["house_location"]["long"].is<float>()
			|| !data["min_distance"].is<float>()) {
			return 400;
		}

		SettingsNVS.setFloat("lat", data["house_location"]["lat"]);
		SettingsNVS.setFloat("lon", data["house_location"]["long"]);
		SettingsNVS.setFloat("trigDist", data["min_distance"]);
		fireAPI.setCoords(SettingsNVS.getFloat("lat"),
						  SettingsNVS.getFloat("lon"));
		fireAPI.setTriggerDistance(SettingsNVS.getFloat("trigDist"));
		eventsObject.queueUp("settings");
		return 201;
	} else if (event == "control") {
		if (!data["activate"].is<bool>()) { return 400; }

		flaps.activate(data["activate"]);
		eventsObject.queueUp("status");
		return 201;
	} else {
		return 404;
	}
}

void initFS() {
	if (!LittleFS.begin(true)) {
		Serial.println("An Error has occurred while mounting LittleFS");
		return;
	}
	if (!LittleFS.exists("/logs.csv")) {
		File logs = LittleFS.open("/logs.csv", "w");
		logs.println("date,time,action,trigger");
		logs.close();
	}
	SettingsNVS.begin("settings");
}

void setup() {
	Serial.begin(115200);
	Serial.println("boot");

	initFS();

	WifiInit(getData, processWebRequest);
	delay(1000);

	fireAPI.setCoords(SettingsNVS.getFloat("lat"), SettingsNVS.getFloat("lon"));
	fireAPI.setTriggerDistance(SettingsNVS.getFloat("trigDist"));

	// flaps.init(12, 13);
}

void loop() {
	eventsObject.tick();
	fireAPI.tick();
}