#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>	// Webserver
#include <WiFi.h>
#include "AsyncJson.h"
#include "LittleFS.h"  // Filesystem

#include "SECRETS.h"
#include "flaps.h"

#include "FireAPI.h"

flaps flaps;

AsyncWebServer server(80);
AsyncEventSource events("/events");	 // event source (Server-Sent events)

static void onRequest(AsyncWebServerRequest* request) {
	// Handle Unknown Request
	request->send(404);
}

static void onBody(AsyncWebServerRequest* request,
				   uint8_t* data,
				   size_t len,
				   size_t index,
				   size_t total) {
	// Handle body
}

void WifiInit() {
	if (!LittleFS.begin(true)) {
		Serial.println("An Error has occurred while mounting LittleFS");
		return;
	}
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	if (WiFi.waitForConnectResult() != WL_CONNECTED) {
		Serial.println("WiFi Failed!\n");
		return;
	}
	Serial.println(WiFi.localIP());

	// attach AsyncEventSource
	server.addHandler(&events);

	// send a file when /index is requested
	server.on("/index", HTTP_ALL, [](AsyncWebServerRequest* request) {
		request->send(LittleFS, "/index.html");
	});

	server.on("/api/settings", HTTP_POST,
			  [](AsyncWebServerRequest* request, JsonVariant& json) {
				  Serial.printf("Body request : ");
				  serializeJson(json, Serial);
				  Serial.println();
				  AsyncJsonResponse* response = new AsyncJsonResponse();
				  JsonObject root = response->getRoot().to<JsonObject>();
				  root["hello"]	  = json.as<JsonObject>()["name"];
				  response->setLength();
				  request->send(response);
			  });

	server.on("/api/control", HTTP_POST,
			  [](AsyncWebServerRequest* request, JsonVariant& json) {
				  Serial.printf("Body request : ");
				  serializeJson(json, Serial);
				  Serial.println();
				  AsyncJsonResponse* response = new AsyncJsonResponse();
				  JsonObject root = response->getRoot().to<JsonObject>();
				  root["hello"]	  = json.as<JsonObject>()["name"];
				  response->setLength();
				  request->send(response);
			  });

	// attach filesystem root at URL /
	server.serveStatic("/", LittleFS, "/");

	// Catch-All Handlers
	// Any request that can not find a Handler that canHandle it
	// ends in the callbacks below.
	server.onNotFound(onRequest);
	server.onRequestBody(onBody);

	server.begin();
}

void setup() {
	Serial.begin(115200);
	Serial.println("boot");
	WifiInit();
	flaps.init(12, 13);
}

void loop() {}