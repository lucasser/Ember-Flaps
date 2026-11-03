"""Relay-style ESP32 dashboard. Demo by default; optional ESP32 HTTP bridge.

ESP32 mode assumes firmware providing:
 GET  /status -> {"connected":true,"motor_on":false}
 POST /motor -> {"on":true|false} then JSON {"motor_on":...}
Change bridge methods to match your ESP32 firmware before enabling it.
"""
import os
from datetime import datetime
from threading import Lock
from urllib.error import URLError, HTTPError
from urllib.request import Request, urlopen
import json
from flask import Flask, jsonify, render_template, request

app = Flask(__name__)
MODE = os.getenv("CONTROL_MODE", "demo").lower()
ESP32_URL = os.getenv("ESP32_URL", "").rstrip("/")
lock = Lock()
state = {"motor_on": False, "activity": [], "last_response": None}


def event(label, detail):
    state["activity"].insert(0, {"time": datetime.now().strftime("%H:%M:%S"), "label": label, "detail": detail})
    del state["activity"][8:]


def call_esp32(path, method="GET", payload=None):
    if not ESP32_URL:
        raise ConnectionError("Set ESP32_URL before using ESP32 mode")
    body = json.dumps(payload).encode() if payload is not None else None
    req = Request(ESP32_URL + path, data=body, method=method,
                  headers={"Content-Type": "application/json"})
    try:
        with urlopen(req, timeout=2.5) as response:
            return json.loads(response.read().decode())
    except (URLError, HTTPError, TimeoutError, ValueError) as exc:
        raise ConnectionError("ESP32 did not respond with valid JSON") from exc


@app.get("/")
def home():
    return render_template("index.html", mode=MODE)


@app.get("/api/status")
def get_status():
    with lock:
        activity = list(state["activity"])
        demo_motor = state["motor_on"]
        last_response = state["last_response"]
    if MODE == "demo":
        return jsonify(mode="demo", connected=False, motor_on=demo_motor,
                       last_response=last_response, activity=activity,
                       message="Simulation only — no physical controller connected")
    if MODE != "esp32":
        return jsonify(error="Unknown CONTROL_MODE"), 500
    try:
        payload = call_esp32("/status")
        motor_on = payload.get("motor_on")
        if not isinstance(motor_on, bool):
            raise ValueError("Missing boolean motor_on")
        connected = payload.get("connected") is True
        if not connected:
            raise ConnectionError("ESP32 reported disconnected")
        return jsonify(mode="esp32", connected=True, motor_on=motor_on,
                       last_response=datetime.now().strftime("%H:%M:%S"),
                       activity=activity, message="ESP32 responded")
    except (ConnectionError, ValueError):
        return jsonify(mode="esp32", connected=False, motor_on=None,
                       last_response=last_response, activity=activity,
                       message="ESP32 not responding")


@app.post("/api/motor")
def motor():
    data = request.get_json(silent=True) or {}
    desired = data.get("on")
    if not isinstance(desired, bool):
        return jsonify(error="Send JSON with boolean 'on'"), 400
    if MODE == "demo":
        with lock:
            state["motor_on"] = desired
            state["last_response"] = datetime.now().strftime("%H:%M:%S")
            event("Motor turned on" if desired else "Motor turned off", "Demo state changed — hardware not connected")
            activity = list(state["activity"])
        return jsonify(mode="demo", connected=False, motor_on=desired,
                       activity=activity, message="Demo command simulated")
    if MODE != "esp32":
        return jsonify(error="Unknown CONTROL_MODE"), 500
    try:
        payload = call_esp32("/motor", "POST", {"on": desired})
        confirmed = payload.get("motor_on")
        if not isinstance(confirmed, bool) or confirmed != desired:
            return jsonify(error="ESP32 did not confirm requested state"), 502
        with lock:
            event("Motor turned on" if confirmed else "Motor turned off", "ESP32 reported output state")
            state["last_response"] = datetime.now().strftime("%H:%M:%S")
            activity = list(state["activity"])
        return jsonify(mode="esp32", connected=True, motor_on=confirmed,
                       activity=activity, message="ESP32 reported output state")
    except ConnectionError:
        return jsonify(error="Cannot contact ESP32; state is unknown"), 503


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5001, debug=False)
