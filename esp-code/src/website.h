#ifndef WEBSITE_H
#define WEBSITE_H

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>	// Webserver
#include <WiFi.h>
#include <queue>
#include "AsyncJson.h"
#include "LittleFS.h"  // Filesystem

#include "SECRETS.h"

inline void test() {}

struct Events {
		struct EventData {
				String event;
				String data;
		};

		std::queue<EventData> eventQueue;
		std::queue<String> eventsToProcess;

		AsyncEventSource* events;

		String (*eventsCallback)(String);

		bool test;

		void tick() {
			if (!eventsToProcess.empty()) {
				queueUp(eventsToProcess.front());
				eventsToProcess.pop();
			}
			if (!eventQueue.empty()) {
				Serial.print("Event: ");
				Serial.print(eventQueue.front().event);
				Serial.print(", Message:");
				Serial.println(eventQueue.front().data);
				events->send(eventQueue.front().data, eventQueue.front().event,
							 millis());
				eventQueue.pop();
			}
		}

		void queueUp(String event, String data) {
			eventQueue.push({event, data});
		}

		void queueUp(String event) { queueUp(event, eventsCallback(event)); }

		void EventsSetup(AsyncEventSource* events,
						 String (*eventsCallback)(String)) {
			this->events		 = events;
			this->eventsCallback = eventsCallback;
			auto eventQueue		 = &this->eventsToProcess;
			events->onConnect([eventQueue](AsyncEventSourceClient* client) {
				if (client->lastId()) {
					Serial.printf(
						"Client reconnected! Last message ID that it got is: "
						"%u\n",
						client->lastId());
				}
				// send event with message "Connected", id current millis
				//  and set reconnect delay to 1 second
				eventQueue->push("settings");
				eventQueue->push("status");
				eventQueue->push("logs");
				eventQueue->push("data");
				client->send("Connected", NULL, millis(), 1000);
			});
		}
};

inline AsyncWebServer server(80);
inline AsyncEventSource events("/events");	// event source (Server-Sent events)

inline Events eventsObject;

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

inline void WifiInit(String (*eventsCallback)(String),
					 int (*postRequestCallback)(String, JsonVariant&)) {
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	if (WiFi.waitForConnectResult() != WL_CONNECTED) {
		Serial.println("WiFi Failed!\n");
		return;
	}
	configTzTime("GMT0", "nz.pool.ntp.org");
	Serial.println(WiFi.localIP());

	// attach AsyncEventSource
	server.addHandler(&events);

	// send a file when /index is requested
	server.on("/", HTTP_ALL, [](AsyncWebServerRequest* request) {
		request->send(LittleFS, "/index.html");
	});

	server.on("/api/settings", HTTP_POST,
			  [postRequestCallback](AsyncWebServerRequest* request,
									JsonVariant& json) {
				  Serial.printf("settings body request : ");
				  serializeJson(json, Serial);
				  Serial.println();
				  request->send(postRequestCallback("settings", json));
			  });

	server.on("/api/control", HTTP_POST,
			  [postRequestCallback](AsyncWebServerRequest* request,
									JsonVariant& json) {
				  Serial.printf("control body request : ");
				  serializeJson(json, Serial);
				  Serial.println();
				  request->send(postRequestCallback("control", json));
			  });

	// attach filesystem root at URL /
	server.serveStatic("/", LittleFS, "/");

	// Catch-All Handlers
	// Any request that can not find a Handler that canHandle it
	// ends in the callbacks below.
	server.onNotFound(onRequest);
	server.onRequestBody(onBody);

	server.begin();

	eventsObject.EventsSetup(&events, eventsCallback);
}

#endif