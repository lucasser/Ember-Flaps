
const $ = (id) => document.getElementById(id);

let currentState = null;
let busy = false;

function setDot(id, connected) {
  $(id).classList.toggle('connected', connected);
  $(id).classList.toggle('disconnected', !connected);
}

function renderActivity(items) {
  const list = $('activity-list');
  list.replaceChildren();

  if (!items || !items.length) {
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
      ['activity-time', item.time],
      ['activity-name', item.label],
      ['activity-detail', item.detail],
      ['activity-confirm', '✓ Reported']
    ];

    for (const [className, value] of fields) {
      const span = document.createElement('span');
      span.className = className;
      span.textContent = value;
      row.append(span);
    }

    list.append(row);
  }
}

function render(s) {
  currentState = s;

  const demo = s.mode === 'demo';
  const connected = s.connected === true;
  const known = typeof s.flaps_engaged === 'boolean';

  $('demo-banner').hidden = !demo;

  $('header-connection').textContent = demo
    ? 'Demo mode'
    : connected
      ? '1 device connected'
      : 'Device disconnected';

  setDot('header-dot', connected);
  setDot('updated-dot', connected);

  $('updated').textContent = connected
    ? 'Last response received'
    : demo
      ? 'Simulated locally'
      : 'Controller unavailable';

  $('connection-state').textContent = demo
    ? 'Demo mode'
    : connected
      ? 'Connected'
      : 'Disconnected';

  document.querySelector('.connection-head')
    .classList.toggle('not-connected', !connected);

  $('connection-description').textContent = demo
    ? 'This is a simulated flap state. No ESP32 has been detected.'
    : connected
      ? 'Your controller responded to the latest status request.'
      : 'No response from ESP32. Check power and Wi-Fi.';

  $('last-response').textContent = s.last_response || '—';

  $('connection-motor').textContent = known
    ? (s.flaps_engaged ? 'ENGAGED' : 'DISENGAGED')
    : 'UNKNOWN';

  $('motor-state').textContent = known
    ? (s.flaps_engaged ? 'ENGAGED' : 'DISENGAGED')
    : 'UNKNOWN';

  $('motor-description').textContent = known
    ? (s.flaps_engaged
        ? 'Flaps commanded closed — protection engaged'
        : 'Flaps commanded open — protection disengaged')
    : 'Flap state cannot be verified';

  document.querySelector('.motor-display')
    .classList.toggle('is-on', s.flaps_engaged === true);

  $('action-title').textContent = known
    ? (s.flaps_engaged ? 'Protection engaged' : 'Ready to deploy')
    : 'Waiting for controller';

  $('action-detail').textContent = demo
    ? 'Simulation mode — no physical flaps will move.'
    : connected
      ? 'Use the button to engage or disengage the flaps.'
      : 'Button disabled until the ESP32 responds.';

  const button = $('motor-button');

  button.disabled = busy || (!demo && !connected) || !known;

  button.classList.toggle(
    'off-button',
    s.flaps_engaged === true
  );

  $('motor-button-label').textContent = busy
    ? 'Sending command...'
    : known
      ? (s.flaps_engaged
          ? 'Disengage flaps'
          : 'Engage flaps')
      : 'Controller unavailable';

  $('motor-button-help').textContent = demo
    ? 'Simulation only — no hardware command'
    : connected
      ? 'Sends command to ESP32-01'
      : 'Connect ESP32 to enable control';

  renderActivity(s.activity);
}

// Refresh ESP32 connection and flap status
async function refresh() {
  if (busy) return;

  try {
    const res = await fetch('/api/status', {
      cache: 'no-store'
    });

    if (!res.ok) {
      throw Error('Server unavailable');
    }

    render(await res.json());

  } catch (err) {
    render({
      mode: 'esp32',
      connected: false,
      flaps_engaged: null,
      activity: currentState?.activity || [],
      message: err.message
    });
  }
}

// BIG BUTTON — ENGAGE / DISENGAGE FLAPS
$('motor-button').addEventListener('click', async () => {
  if (
    busy ||
    !currentState ||
    typeof currentState.flaps_engaged !== 'boolean'
  ) return;

  const demo = currentState.mode === 'demo';

  if (!demo && currentState.connected !== true) return;

  // Toggle current flap state
  // true = closed / engaged
  // false = open / disengaged
  const desired = !currentState.flaps_engaged;

  const confirmed = confirm(
    desired
      ? `ENGAGE protection and CLOSE the flaps${demo ? ' in DEMO MODE' : ''}?`
      : `DISENGAGE protection and OPEN the flaps${demo ? ' in DEMO MODE' : ''}?`
  );

  if (!confirmed) return;

  busy = true;
  $('motor-button').disabled = true;
  $('motor-button-label').textContent = 'Sending command...';

  try {
    // Send boolean to teammate's do_flaps() endpoint
    const res = await fetch('/api/do_flaps', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({
        engaged: desired
      })
    });

    if (!res.ok) {
      let errorMessage = 'Command failed';

      try {
        const data = await res.json();
        errorMessage = data.error || errorMessage;
      } catch (_) {}

      throw Error(errorMessage);
    }

    console.log(
      'Flap command accepted:',
      desired ? 'ENGAGED' : 'DISENGAGED'
    );

  } catch (err) {
    console.error('Flap command error:', err);
    alert('Command not confirmed: ' + err.message);

  } finally {
    busy = false;
    await refresh();
  }
});

// Initial connection check
refresh();

// Automatically refresh status every 4 seconds
setInterval(refresh, 4000);
