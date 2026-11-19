#ifndef FireAPI_H
#define FireAPI_H

#include <Arduino.h>
#include <esp_http_client.h>

class FireAPI {
	private:
		double lat;
		double lon;

		double triggerDistance;

	public:
		void setCoords(double lat, double lon) {
			this->lat = lat;
			this->lon = lon;
		}

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