#ifndef FireAPI_H
#define FireAPI_H

#define CSV_PARSER_DONT_IMPORT_SD
#include <Arduino.h>
#include <ArduinoJson.h>
#include <CSV_Parser.h>
#include <HTTPClient.h>

#include "WString.h"

#include "website.h"

/*
latitude,longitude,bright_ti4,scan,track,acq_date,acq_time,satellite,instrument,confidence,version,bright_ti5,frp,daynight
47.29781,-103.03623,301.5,0.52,0.42,2026-11-15,837,N20,VIIRS,n,2.0NRT,270.84,0.73,N
*/

class FireAPI {
	private:
		struct Settings {
				double lat;
				double lon;
				double triggerDistance;
				double detectionDistance;

				String API_KEY;
		};

		const unsigned long FETCH_DATA_INTERVAL = 300000;

		Settings settings;

		JsonDocument data;

		unsigned long lastFetch;

	public:
		FireAPI(Settings settings): settings(settings) {}

		void setCoords(double lat, double lon) {
			settings.lat = lat;
			settings.lon = lon;
			tick(true);
		}

		void setTriggerDistance(double distance) {
			settings.triggerDistance   = distance;
			settings.detectionDistance = distance * 5;
		}

		void tick(bool force = false) {
			if (millis() - lastFetch > FETCH_DATA_INTERVAL || force) {
				fetchData();
				eventsObject.queueUp("data");
				lastFetch = millis();
			}
		}

		void fetchData() {
			if (WiFi.status() != WL_CONNECTED) {
				Serial.println("DATA_UNAVAILABLE: Wi-Fi disconnected");
				return;
			}
			CSV_Parser cp(/*format*/ "ff---ss-------");

			String area
				= String(settings.lon - settings.detectionDistance) + ","
				  + String(settings.lat - settings.detectionDistance) + ","
				  + String(settings.lon + settings.detectionDistance) + ","
				  + String(settings.lat + settings.detectionDistance);

			String path = "https://firms.modaps.eosdis.nasa.gov/api/area/csv/"
						  + settings.API_KEY + "/VIIRS_NOAA20_NRT/" + area
						  + "/2";
			Serial.println(path);
			HTTPClient http;

			http.setInsecure();
			http.begin(path);

			int httpResponseCode = http.GET();

			if (httpResponseCode > 0) {
				Serial.print("HTTP Response code: ");
				Serial.println(httpResponseCode);
				// http.writeToStream(new CSV_Stream(&cp));
				cp << http.getString();
			} else {
				Serial.print("Error code: ");
				Serial.println(httpResponseCode);
				http.end();
			}
			// Free resources
			http.end();

			float* lat	= (float*)cp["latitude"];
			float* lon	= (float*)cp["longitude"];
			char** date = (char**)cp["acq_date"];
			char** time = (char**)cp["acq_time"];

			JsonDocument fireData;
			JsonDocument fire;
			JsonArray fireArray = fireData["fires"].to<JsonArray>();

			for (int i = 0; i < cp.getRowsCount(); i++) {
				fire["lat"]	 = lat[i];
				fire["lon"]	 = lon[i];
				fire["date"] = date[i];
				fire["time"] = time[i];

				fireArray.add(fire);
			}
			data = fireData;
		}

		JsonDocument getData() { return data; }

	private:
		double distanceMiles(double lat1,
							 double lon1,
							 double lat2,
							 double lon2) {
			const double R	 = 3958.8;
			const double DEG = PI / 180.0;

			double dLat = (lat2 - lat1) * DEG;
			double dLon = (lon2 - lon1) * DEG;

			double a
				= pow(sin(dLat / 2), 2)
				  + cos(lat1 * DEG) * cos(lat2 * DEG) * pow(sin(dLon / 2), 2);

			a = constrain(a, 0.0, 1.0);

			return 2 * R * atan2(sqrt(a), sqrt(1 - a));
		}
};

#endif