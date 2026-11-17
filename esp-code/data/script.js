const $ = (id) => document.getElementById(id);

let currentState = null;
let flapBusy = false;
let coordBusy = false;
let statusRequestPending = false;
let coordinatesInitialized = false;
let locationSubmitted = false;

function renderActivity(items) {
  const list = $('activity-list');
  list.replaceChildren();

  if (!Array.isArray(items) || items.length === 0) {
    const row = document.createElement('div');
    row.className = 'empty-activity';
    row.textContent = 'No activity recorded yet.';
    list.append(row);
    return;
  }

  for (const item of items) {
    const row = document.createElement('div');
    row.className = 'activity-row';
    const fields = [
      ['activity-time', item.time ?? '—'],
      ['activity-name', item.label ?? 'Activity'],
      ['activity-detail', item.detail ?? ''],
      ['activity-confirm', '✓ Reported']
    ];

    for (const [className, value] of fields) {
      const span = document.createElement('span');
      span.className = className;
      span.textContent = String(value);
      row.append(span);
    }
    list.append(row);
  }
}

function render(s, statusAvailable = true) {
  currentState = s;
  const known = statusAvailable && typeof s.flaps_engaged === 'boolean';
  const engaged = known && s.flaps_engaged;

  $('updated').textContent = statusAvailable
    ? `Status checked ${new Date().toLocaleTimeString([], { hour: 'numeric', minute: '2-digit' })}`
    : 'Status temporarily unavailable';

  $('motor-state').textContent = known
    ? (engaged ? 'ENGAGED' : 'DISENGAGED')
    : 'UNKNOWN';

  $('motor-description').textContent = known
    ? (engaged
        ? 'Close command reported — physical position unverified.'
        : 'Open command reported — physical position unverified.')
    : 'Flap command state is unavailable.';

  document.querySelector('.motor-display').classList.toggle('is-on', engaged);

  $('action-title').textContent = known
    ? (engaged ? 'Close command active' : 'Ready to engage')
    : 'Waiting for flap status';

  $('action-detail').textContent = known
    ? 'Use the button to engage or disengage the flaps.'
    : 'The button is disabled until a valid flap state is received.';

  const motorButton = $('motor-button');
  motorButton.disabled = flapBusy || !known;
  motorButton.classList.toggle('off-button', engaged);

  $('motor-button-label').textContent = flapBusy
    ? 'Sending command…'
    : known
      ? (engaged ? 'Disengage flaps' : 'Engage flaps')
      : 'Status unavailable';

  $('motor-button-help').textContent = known
    ? 'Sends a command to the flap system'
    : 'Awaiting a valid flap state';

  // The API is expected to return a distance in miles. No value is invented.
  const distance = s.nearest_fire_distance_miles;
  const validDistance = statusAvailable &&
    typeof distance === 'number' && Number.isFinite(distance) && distance >= 0;

  $('fire-unit').hidden = !validDistance;

  if (validDistance) {
    $('fire-distance').textContent = distance < 10
      ? distance.toFixed(1)
      : Math.round(distance).toLocaleString();
    $('fire-description').textContent = 'Distance to the nearest reported fire from the saved location.';
  } else {
    $('fire-distance').textContent = '—';
    $('fire-description').textContent = statusAvailable
      ? (locationSubmitted || s.coord_input
          ? 'No fire distance is available yet for this location.'
          : 'Enter your coordinates to calculate a distance.')
      : 'Fire distance is currently unavailable.';
  }

  // Pre-fill once from coordinates returned by the server, if available.
  const saved = s.coord_input;
  if (!coordinatesInitialized && saved &&
      typeof saved.latitude === 'number' &&
      typeof saved.longitude === 'number' &&
      Number.isFinite(saved.latitude) && Number.isFinite(saved.longitude)) {
    $('coord-latitude').value = String(saved.latitude);
    $('coord-longitude').value = String(saved.longitude);
    coordinatesInitialized = true;
    locationSubmitted = true;
    $('coord-feedback').textContent = 'Location loaded from the system.';
  }

  renderActivity(s.activity);
}

async function refresh() {
  if (flapBusy || statusRequestPending) return;
  statusRequestPending = true;

  try {
    const res = await fetch('/api/status', { cache: 'no-store' });
    if (!res.ok) throw new Error('Status request failed');
    const data = await res.json();
    render(data, true);
  } catch (err) {
    console.error('Status error:', err);
    render({
      flaps_engaged: null,
      nearest_fire_distance_miles: null,
      activity: currentState?.activity || []
    }, false);
  } finally {
    statusRequestPending = false;
  }
}

// Big button: true = CLOSED / ENGAGED; false = OPEN / DISENGAGED.
$('motor-button').addEventListener('click', async () => {
  if (flapBusy || !currentState ||
      typeof currentState.flaps_engaged !== 'boolean') return;

  const engaged = !currentState.flaps_engaged;
  const confirmed = confirm(engaged
    ? 'Engage protection and CLOSE the flaps?'
    : 'Disengage protection and OPEN the flaps?');
  if (!confirmed) return;

  flapBusy = true;
  $('motor-button').disabled = true;
  $('motor-button-label').textContent = 'Sending command…';

  try {
    const res = await fetch('/api/do_flaps', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ engaged })
    });

    const data = await res.json().catch(() => ({}));
    if (!res.ok || data.success === false) {
      throw new Error(data.error || 'Flap command failed');
    }
    // The status endpoint is the source of truth; do not assume motion happened.
  } catch (err) {
    console.error('Flap command error:', err);
    alert('Command not confirmed: ' + err.message);
  } finally {
    flapBusy = false;
    await refresh();
  }
});

// User coordinates sent under the exact JSON key "coord_input".
$('coord_input').addEventListener('submit', async (event) => {
  event.preventDefault();
  if (coordBusy) return;

  const latitudeText = $('coord-latitude').value.trim();
  const longitudeText = $('coord-longitude').value.trim();
  const latitude = Number(latitudeText);
  const longitude = Number(longitudeText);

  if (!latitudeText || !longitudeText ||
      !Number.isFinite(latitude) || latitude < -90 || latitude > 90 ||
      !Number.isFinite(longitude) || longitude < -180 || longitude > 180) {
    $('coord-feedback').textContent = 'Enter valid latitude (−90 to 90) and longitude (−180 to 180).';
    $('coord-feedback').classList.add('is-error');
    return;
  }

  const coord_input = { latitude, longitude };
  coordBusy = true;
  $('coord-submit').disabled = true;
  $('coord-submit').textContent = 'Saving location…';
  $('coord-feedback').classList.remove('is-error');
  $('coord-feedback').textContent = 'Sending your coordinates…';
  // A previous property's fire distance must not be shown as the new one.
  $('fire-distance').textContent = '—';
  $('fire-unit').hidden = true;
  $('fire-description').textContent = 'Calculating for the updated location…';

  try {
    const res = await fetch('/api/coord_input', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ coord_input })
    });

    const data = await res.json().catch(() => ({}));
    if (!res.ok || data.success === false) {
      throw new Error(data.error || 'Could not save coordinates');
    }

    locationSubmitted = true;
    coordinatesInitialized = true;
    $('coord-feedback').textContent = 'Location saved. Updating nearest-fire distance…';
    await refresh();
  } catch (err) {
    console.error('Location update error:', err);
    $('coord-feedback').textContent = 'Location not saved: ' + err.message;
    $('coord-feedback').classList.add('is-error');
    await refresh(); // Re-display server's still-saved location/distance.
  } finally {
    coordBusy = false;
    $('coord-submit').disabled = false;
    $('coord-submit').textContent = 'Update location';
  }
});

refresh();
setInterval(refresh, 4000);
