# Communication API

## Server -> Client

S->C comms will be done using [EventSource](https://developer.mozilla.org/en-US/docs/Web/API/EventSource)
Will receive text
Each of these event will be sent on initial connection with current info

- flap status
  - Event name: status
  - Data: text
    - open or closed
- fire data
  - distance to fires to display to user should be calculated on client (the logic for operating flaps will be done on server)
  - Event name: data
  - Data: json as text:

```json
{
    "fires": [
        {
            "lat": "double",
            "long": "double",
            "detection_date": "year-mon-day (2026-11-15)",
            "detection_time": "HMM (800), UTC time"
        }
    ]
}
```

- Settings
  - Event name: settings
  - Data: json as text:

```json
{
    "house_location": {
        "lat": "double",
        "long": "double" 
    },
    "min_distance": "double (in miles)"
}
```

- flap activation history
  - Event name: logs
  - Data: json as text

```json
{
    "date": "year-mon-day (2026-11-15)",
    "time": "HMM (800), UTC time",
    "action": "on/off",
    "trigger": "User, timer, fire too close..."
}
```

## Client -> Server

C->S comms will be using post requests with json objects to an api url

- activate flaps
  - Listening on /api/control
  - Json format:

```json
{
    "activate": true/false
}
```

- settings
  - Listening on /api/settings
  - Json format:

```json
{
    "house_location": {
        "lat": "double",
        "long": "double" 
    },
    "min_distance": "double (in miles)"
}
```