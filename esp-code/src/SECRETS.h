#include <WString.h>

inline static const char* ssid	   = "IslandiaNet";
inline static const char* password = "wifiwifi";

inline static const String API_KEY = "fb25099c6cd8c13387b17400d89568f3";

inline static const String API_URL
	= "https://firms.modaps.eosdis.nasa.gov/api/area/csv/" + API_KEY
	  + "/VIIRS_NOAA20_NRT/";