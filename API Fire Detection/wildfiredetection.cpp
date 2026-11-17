
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <time.h>
#include <math.h>

// ---- WIFI SETTINGS ----
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// ---- NASA FIRMS SETTINGS ----
const char* MAP_KEY = "YOUR_MAP_KEY";
const char* AREA = "-105.5,39.5,-104.8,40.1";

// ---- HOUSE LOCATION ----
const double HOUSE_LAT = 39.7555;
const double HOUSE_LON = -105.2211;

// ---- ACTIVATION SETTINGS ----
const double TRIGGER_RADIUS_MILES = 10.0;
const int MAX_DETECTION_AGE_HOURS = 6;
const unsigned long CHECK_INTERVAL = 300000UL;

// ---- MOTOR REQUEST OUTPUT ----
// NEW: GPIO pin used to signal the motor controller
const int MOTOR_REQUEST_PIN = 25;

// Deployment request stays active until reset
bool deploymentRequested = false;
unsigned long lastCheck = 0;

double distanceMiles(double lat1, double lon1,
                     double lat2, double lon2) {
  const double R = 3958.8;
  const double DEG = PI / 180.0;

  double dLat = (lat2 - lat1) * DEG;
  double dLon = (lon2 - lon1) * DEG;

  double a = pow(sin(dLat / 2), 2) +
             cos(lat1 * DEG) * cos(lat2 * DEG) *
             pow(sin(dLon / 2), 2);

  a = constrain(a, 0.0, 1.0);

  return 2 * R * atan2(sqrt(a), sqrt(1 - a));
}

// Extract one field from a comma-separated row.
// Appropriate for NASA's numeric/date fields.
String csvField(const String& line, int index) {
  int start = 0;

  for (int i = 0; i < index; i++) {
    int comma = line.indexOf(',', start);
    if (comma < 0) return "";
    start = comma + 1;
  }

  int end = line.indexOf(',', start);
  if (end < 0) end = line.length();

  String value = line.substring(start, end);
  value.trim();
  return value;
}

// Convert NASA's UTC date/time to Unix time
time_t detectionTime(String date, String hhmm) {
  int year, month, day, hourMinute;

  if (sscanf(date.c_str(), "%d-%d-%d",
             &year, &month, &day) != 3)
    return -1;

  if (sscanf(hhmm.c_str(), "%d", &hourMinute) != 1)
    return -1;

  struct tm t = {};
  t.tm_year = year - 1900;
  t.tm_mon = month - 1;
  t.tm_mday = day;
  t.tm_hour = hourMinute / 100;
  t.tm_min = hourMinute % 100;

  if (t.tm_hour > 23 || t.tm_min > 59)
    return -1;

  return timegm(&t);
}

void checkFires() {
  Serial.println("\nChecking NASA FIRMS...");

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("DATA_UNAVAILABLE: Wi-Fi disconnected");
    return;
  }

  // Confirm the ESP32 clock has synchronized
  time_t now = time(nullptr);
  if (now < 1700000000) {
    Serial.println("DATA_UNAVAILABLE: Clock not ready");
    return;
  }

  String url =
      String("https://firms.modaps.eosdis.nasa.gov")
      + "/api/area/csv/"
      + MAP_KEY
      + "/VIIRS_NOAA20_NRT/"
      + AREA + "/1";

  WiFiClientSecure client;

  // PROTOTYPE ONLY: disables certificate verification.
  // Replace with trusted CA verification before deployment.
  client.setInsecure();

  HTTPClient http;
  http.setTimeout(20000);

  if (!http.begin(client, url)) {
    Serial.println("DATA_UNAVAILABLE: HTTP setup failed");
    return;
  }

  int status = http.GET();

  if (status != 200) {
    Serial.printf("DATA_UNAVAILABLE: HTTP %d\n", status);
    http.end();
    return;
  }

  String csv = http.getString();
  http.end();

  // Check expected CSV header
  if (!csv.startsWith("latitude,longitude,")) {
    Serial.println("DATA_UNAVAILABLE: Invalid CSV");
    return;
  }

  int pos = csv.indexOf('\n') + 1;
  int validDetections = 0;
  double nearest = 1e9;
  bool nearbyFire = false;

  while (pos > 0 && pos < csv.length()) {
    int end = csv.indexOf('\n', pos);
    if (end < 0) end = csv.length();

    String row = csv.substring(pos, end);
    row.trim();
    pos = end + 1;

    if (row.length() == 0) continue;

    // NASA VIIRS columns:
    // 0 latitude, 1 longitude,
    // 5 acq_date, 6 acq_time
    String latStr = csvField(row, 0);
    String lonStr = csvField(row, 1);
    String dateStr = csvField(row, 5);
    String timeStr = csvField(row, 6);

    if (latStr.length() == 0 ||
        lonStr.length() == 0 ||
        dateStr.length() == 0 ||
        timeStr.length() == 0) {
      Serial.println("DATA_UNAVAILABLE: Invalid row");
      return;
    }

    double lat = latStr.toDouble();
    double lon = lonStr.toDouble();

    time_t detected = detectionTime(dateStr, timeStr);
    if (detected < 0) {
      Serial.println("DATA_UNAVAILABLE: Bad timestamp");
      return;
    }

    double ageHours = difftime(now, detected) / 3600.0;

    if (ageHours < 0 ||
        ageHours > MAX_DETECTION_AGE_HOURS)
      continue;

    validDetections++;

    double distance = distanceMiles(
        HOUSE_LAT, HOUSE_LON, lat, lon);

    if (distance < nearest) nearest = distance;

    if (distance <= TRIGGER_RADIUS_MILES) {
      nearbyFire = true;
    }
  }

  Serial.printf("Recent detections: %d\n", validDetections);

  if (validDetections > 0) {
    Serial.printf("Nearest: %.2f miles\n", nearest);
  }

  if (nearbyFire) {
    deploymentRequested = true;
    Serial.println("ACTIVATE: Nearby fire detected");
  } else {
    Serial.println("NO_API_TRIGGER");
  }

  if (deploymentRequested) {
    Serial.println("DEPLOYMENT REQUEST LATCHED");

    // NEW: Output 3.3V on GPIO 25
    digitalWrite(MOTOR_REQUEST_PIN, HIGH);

    // NEW: Confirm electrical output in Serial Monitor
    Serial.println("GPIO 25: HIGH (3.3V requested)");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // NEW: Configure GPIO 25 as an output
  pinMode(MOTOR_REQUEST_PIN, OUTPUT);

  // NEW: Start with GPIO 25 OFF (0V)
  digitalWrite(MOTOR_REQUEST_PIN, LOW);

  Serial.println("Motor request output initialized: LOW");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");

  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - start < 20000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi connected");
  } else {
    Serial.println("\nWi-Fi unavailable");
  }

  // Synchronize clock using internet time servers
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  lastCheck = millis() - CHECK_INTERVAL;
}

void loop() {
  if (millis() - lastCheck >= CHECK_INTERVAL) {
    lastCheck = millis();
    checkFires();
  }
}
