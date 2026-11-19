
const EVENTS_URL = '/api/events';
const $ = (id) => document.getElementById(id);

let flapPosition = null; // 'open' or 'closed' reported by firmware, not sensor proof
let settings = null;     // { house_location: { lat, long }, min_distance }
let fires = null;        // null = not received yet; [] = no reported fires
let recentLogs = [];
let flapBusy = false;
let settingsBusy = false;
let eventStreamLive = false;
let editingCoordinates = false;
let pendingLocation = null;
let propertyMap = null;
let propertyMarker = null;

function coordinate(value, low, high) {
  if (value === null || value === undefined || value === '') return null;
  const result = Number(value);
  return Number.isFinite(result) && result >= low && result <= high ? result : null;
}

function houseCoordinates() {
  if (!settings || !settings.house_location) return null;
  const lat = coordinate(settings.house_location.lat, -90, 90);
  const lon = coordinate(settings.house_location.long, -180, 180);
  return lat === null || lon === null ? null : { lat, lon };
}

// Haversine great-circle distance, in miles (straight-line surface distance).
// This is the CLIENT-SIDE calculation requested by API.md.
function haversineMiles(lat1, lon1, lat2, lon2) {
  const radians = (degrees) => degrees * Math.PI / 180;
  const dLat = radians(lat2 - lat1);
  const dLon = radians(lon2 - lon1);
  const a = Math.sin(dLat / 2) ** 2 +
    Math.cos(radians(lat1)) * Math.cos(radians(lat2)) *
    Math.sin(dLon / 2) ** 2;
  return 2 * 3958.8 * Math.asin(Math.sqrt(Math.min(1, Math.max(0, a))));
}

function renderFlaps() {
  const known = eventStreamLive && (flapPosition === 'open' || flapPosition === 'closed');
  const engaged = flapPosition === 'closed';
  $('motor-state').textContent = known ? (engaged ? 'ENGAGED' : 'DISENGAGED') : 'UNKNOWN';
  $('motor-description').textContent = known
    ? (engaged ? 'Flaps reported closed — physical position not independently verified.'
               : 'Flaps reported open — physical position not independently verified.')
    : 'Waiting for a current flap status.';
  document.querySelector('.motor-display').classList.toggle('is-on', known && engaged);
  $('action-title').textContent = known ? (engaged ? 'Protection engaged' : 'Ready to deploy') : 'Flap status unavailable';
  $('action-detail').textContent = known
    ? 'Use the button to engage or disengage the flaps.'
    : 'The button is disabled until live flap status is available.';
  $('motor-button').disabled = flapBusy || !known;
  $('motor-button').classList.toggle('off-button', known && engaged);
  $('motor-button-label').textContent = flapBusy ? 'Sending command…'
    : known ? (engaged ? 'Disengage flaps' : 'Engage flaps') : 'Status unavailable';
  $('motor-button-help').textContent = known ? 'Sends a command to the flap controller' : 'Awaiting live status';
}

function renderDistance() {
  if (pendingLocation) {
    $('fire-distance').textContent = '—';
    $('fire-unit').hidden = true;
    $('fire-description').textContent = 'Waiting for updated location confirmation…';
    return;
  }
  const home = houseCoordinates();
  const validFires = Array.isArray(fires) ? fires.flatMap((fire) => {
    const lat = coordinate(fire.lat, -90, 90);
    const lon = coordinate(fire.long, -180, 180);
    return lat === null || lon === null ? [] : [{ lat, lon }];
  }) : [];
  if (home && validFires.length) {
    const nearest = Math.min(...validFires.map((fire) =>
      haversineMiles(home.lat, home.lon, fire.lat, fire.lon)));
    $('fire-distance').textContent = nearest < 10 ? nearest.toFixed(1) : nearest.toFixed(1);
    $('fire-unit').hidden = false;
    $('fire-description').textContent = 'Straight-line distance to the nearest reported fire detection.';
  } else {
    $('fire-distance').textContent = '—';
    $('fire-unit').hidden = true;
    $('fire-description').textContent = !home
      ? 'Enter and save a property location to calculate fire distance.'
      : fires === null ? 'Waiting for fire data…'
      : validFires.length === 0 ? 'No valid fire detections were reported.'
      : 'Fire distance unavailable.';
  }
}

function formatUtc(date, time) {
  const t = String(time ?? '').padStart(4, '0');
  return `${date || '—'} ${t.slice(0, 2)}:${t.slice(2)} UTC`;
}

function renderActivity() {
  const list = $('activity-list');
  list.replaceChildren();
  if (!recentLogs.length) {
    const row = document.createElement('div');
    row.className = 'empty-activity';
    row.textContent = 'No activity recorded yet.';
    list.append(row);
    return;
  }
  for (const entry of recentLogs.slice(0, 8)) {
    const row = document.createElement('div');
    row.className = 'activity-row';
    const action = entry.action === 'on' ? 'Flaps engaged'
      : entry.action === 'off' ? 'Flaps disengaged' : 'Flap activity';
    for (const [className, value] of [
      ['activity-time', formatUtc(entry.date, entry.time)],
      ['activity-name', action],
      ['activity-detail', entry.trigger || 'Unspecified trigger'],
      ['activity-confirm', '✓ Reported']
    ]) {
      const span = document.createElement('span');
      span.className = className;
      span.textContent = String(value);
      row.append(span);
    }
    list.append(row);
  }
}

function setMapLocation(lat, lon, moveView = true) {
  if (!propertyMap || typeof L === 'undefined') return;
  if (moveView) propertyMap.setView([lat, lon], Math.max(propertyMap.getZoom(), 13));
  if (propertyMarker) propertyMarker.setLatLng([lat, lon]);
  else propertyMarker = L.marker([lat, lon]).addTo(propertyMap);
}

function renderSettings() {
  const home = houseCoordinates();
  if (!editingCoordinates && home) {
    $('coord-latitude').value = String(home.lat);
    $('coord-longitude').value = String(home.lon);
    setMapLocation(home.lat, home.lon);
  }
  // Preserve the server's min_distance. API.md requires it on every settings POST.
  const ready = settings && home && Number.isFinite(Number(settings.min_distance)) && Number(settings.min_distance) >= 0;
  $('coord-submit').disabled = settingsBusy || !ready;
  if (!settingsBusy) $('coord-submit').textContent = ready ? 'Update location' : 'Waiting for settings…';
}

function updateConnectionDisplay() {
  $('updated').textContent = eventStreamLive ? 'Live updates active' : 'Waiting for live updates…';
  renderFlaps();
}

// Server-sent events: server pushes current values on initial connection.
const stream = new EventSource(EVENTS_URL);
stream.onopen = () => {
  eventStreamLive = true;
  // On reconnection, wait for a fresh status event before enabling controls.
  flapPosition = null;
  updateConnectionDisplay();
};
stream.onerror = () => {
  eventStreamLive = false;
  flapPosition = null;
  updateConnectionDisplay();
};
stream.addEventListener('status', (event) => {
  const value = event.data.trim().toLowerCase();
  if (value !== 'open' && value !== 'closed') return;
  flapPosition = value;
  renderFlaps();
});
stream.addEventListener('data', (event) => {
  try {
    const incoming = JSON.parse(event.data);
    if (!Array.isArray(incoming.fires)) throw Error('Missing fires array');
    fires = incoming.fires;
    renderDistance();
  } catch (err) { console.error('Invalid fire data event:', err); }
});
stream.addEventListener('settings', (event) => {
  try {
    const incoming = JSON.parse(event.data);
    const lat = coordinate(incoming.house_location?.lat, -90, 90);
    const lon = coordinate(incoming.house_location?.long, -180, 180);
    const minDistance = Number(incoming.min_distance);
    if (lat === null || lon === null || !Number.isFinite(minDistance) || minDistance < 0) {
      throw Error('Invalid house_location or min_distance');
    }
    settings = { house_location: { lat, long: lon }, min_distance: minDistance };
    if (pendingLocation && Math.abs(pendingLocation.lat - lat) < 0.000001 &&
        Math.abs(pendingLocation.lon - lon) < 0.000001) {
      pendingLocation = null;
      editingCoordinates = false;
      $('coord-feedback').classList.remove('is-error');
      $('coord-feedback').textContent = 'Location saved and confirmed.';
    }
    renderSettings();
    renderDistance();
  } catch (err) { console.error('Invalid settings event:', err); }
});
stream.addEventListener('logs', (event) => {
  try {
    const incoming = JSON.parse(event.data);
    // Accept a single log (documented) or an array of logs for initial history.
    const entries = Array.isArray(incoming) ? incoming
      : Array.isArray(incoming.logs) ? incoming.logs : [incoming];
    for (const entry of entries) {
      if (!entry || typeof entry !== 'object') continue;
      const key = JSON.stringify([entry.date, entry.time, entry.action, entry.trigger]);
      if (!recentLogs.some((old) => JSON.stringify([old.date, old.time, old.action, old.trigger]) === key)) {
        recentLogs.unshift(entry);
      }
    }
    recentLogs = recentLogs.slice(0, 8);
    renderActivity();
  } catch (err) { console.error('Invalid logs event:', err); }
});

async function postJson(url, payload) {
  const response = await fetch(url, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(payload)
  });
  if (!response.ok) {
    const text = await response.text().catch(() => '');
    throw new Error(`HTTP ${response.status}${text ? `: ${text.slice(0, 150)}` : ''}`);
  }
}

// Boolean API: true = close/engage; false = open/disengage.
$('motor-button').addEventListener('click', async () => {
  if (flapBusy || !eventStreamLive || (flapPosition !== 'open' && flapPosition !== 'closed')) return;
  const activate = flapPosition === 'open';
  if (!confirm(activate ? 'Engage protection and CLOSE the flaps?'
                        : 'Disengage protection and OPEN the flaps?')) return;
  flapBusy = true;
  renderFlaps();
  try {
    await postJson('/api/control', { activate });
    // Do not guess the outcome; next 'status' event is authoritative.
  } catch (err) {
    alert('Flap command not confirmed: ' + err.message);
  } finally {
    flapBusy = false;
    renderFlaps();
  }
});

// The HTML form remains named coord_input; the wire format follows API.md.
$('coord-latitude').addEventListener('input', () => { editingCoordinates = true; });
$('coord-longitude').addEventListener('input', () => { editingCoordinates = true; });
$('coord_input').addEventListener('submit', async (event) => {
  event.preventDefault();
  if (settingsBusy) return;
  const lat = coordinate($('coord-latitude').value.trim(), -90, 90);
  const lon = coordinate($('coord-longitude').value.trim(), -180, 180);
  const minDistance = settings && Number(settings.min_distance);
  if (lat === null || lon === null) {
    $('coord-feedback').textContent = 'Enter a valid latitude (−90 to 90) and longitude (−180 to 180).';
    $('coord-feedback').classList.add('is-error');
    return;
  }
  if (!settings || !Number.isFinite(minDistance) || minDistance < 0) {
    $('coord-feedback').textContent = 'Waiting for the current minimum-distance setting from the system.';
    $('coord-feedback').classList.add('is-error');
    return;
  }
  settingsBusy = true;
  renderSettings();
  $('coord-feedback').classList.remove('is-error');
  $('coord-feedback').textContent = 'Saving property location…';
  pendingLocation = { lat, lon };
  renderDistance();
  try {
    await postJson('/api/settings', {
      house_location: { lat, long: lon },
      min_distance: minDistance
    });
    if (pendingLocation) {
      $('coord-feedback').textContent = 'Location submitted. Waiting for updated settings…';
    }
  } catch (err) {
    pendingLocation = null;
    renderDistance();
    $('coord-feedback').textContent = 'Location not saved: ' + err.message;
    $('coord-feedback').classList.add('is-error');
  } finally {
    settingsBusy = false;
    renderSettings();
  }
});

function selectPropertyLocation(lat, lon, message) {
  if (!Number.isFinite(lat) || !Number.isFinite(lon) || Math.abs(lat) > 90 || Math.abs(lon) > 180) return;
  $('coord-latitude').value = lat.toFixed(6);
  $('coord-longitude').value = lon.toFixed(6);
  editingCoordinates = true;
  setMapLocation(lat, lon);
  $('coord-feedback').classList.remove('is-error');
  $('coord-feedback').textContent = message + ' Press Update location to save.';
}

function initializePropertyMap() {
  const feedback = $('map-feedback');
  if (typeof L === 'undefined') {
    feedback.textContent = 'Map unavailable. You can still enter coordinates manually.';
    return;
  }
  try {
    propertyMap = L.map('property-map').setView([39.74, -105.2], 10);
    L.tileLayer('https://tile.openstreetmap.org/{z}/{x}/{y}.png', {
      maxZoom: 19, attribution: '&copy; OpenStreetMap contributors'
    }).addTo(propertyMap);
    propertyMap.on('click', (event) => {
      selectPropertyLocation(event.latlng.lat, event.latlng.lng, 'Map location selected.');
    });
    if (L.control && typeof L.control.photon === 'function') {
      const search = L.control.photon({
        placeholder: 'Search property address…', position: 'topleft',
        url: 'https://photon.komoot.io/api/'
      });
      search.addTo(propertyMap);
      search.on('selected', (event) => {
        const choice = event.choice;
        const coords = choice?.geometry?.coordinates;
        if (coords && coords.length >= 2) {
          selectPropertyLocation(Number(coords[1]), Number(coords[0]), 'Address selected.');
        } else if (choice?.latlng) {
          selectPropertyLocation(choice.latlng.lat, choice.latlng.lng, 'Address selected.');
        }
      });
      feedback.textContent = 'Search for an address or tap the map, then press Update location.';
    } else feedback.textContent = 'Address search unavailable. Tap the map or enter coordinates below.';
  } catch (err) {
    console.error('Map failed:', err);
    feedback.textContent = 'Map unavailable. Enter coordinates manually below.';
  }
}

initializePropertyMap();
renderFlaps();
renderDistance();
renderActivity();
