const $ = (id) => document.getElementById(id);
let currentState = null;
let busy = false;
function setDot(id, connected) { $(id).classList.toggle('connected', connected); $(id).classList.toggle('disconnected', !connected); }
function renderActivity(items) {
  const list = $('activity-list'); list.replaceChildren();
  if (!items || !items.length) { const row = document.createElement('div'); row.className = 'empty-activity'; row.textContent = 'No activity recorded yet.'; list.append(row); return; }
  for (const item of items) {
    const row = document.createElement('div'); row.className = 'activity-row';
    const fields = [['activity-time', item.time], ['activity-name', item.label], ['activity-detail', item.detail], ['activity-confirm', '✓ Reported']];
    for (const [className, value] of fields) { const span = document.createElement('span'); span.className = className; span.textContent = value; row.append(span); } list.append(row);
  }
}
function render(s) {
  currentState = s;
  const demo = s.mode === 'demo'; const connected = s.connected === true;
  $('demo-banner').hidden = !demo;
  $('header-connection').textContent = demo ? 'Demo mode' : connected ? '1 device connected' : 'Device disconnected';
  setDot('header-dot', connected); setDot('updated-dot', connected);
  $('updated').textContent = connected ? 'Last response received' : demo ? 'Simulated locally' : 'Controller unavailable';
  $('connection-state').textContent = demo ? 'Demo mode' : connected ? 'Connected' : 'Disconnected';
  document.querySelector('.connection-head').classList.toggle('not-connected', !connected);
  $('connection-description').textContent = demo ? 'This is a simulated motor state. No ESP32 has been detected.' : connected ? 'Your controller responded to the latest status request.' : 'No response from ESP32. Check power and Wi-Fi.';
  $('last-response').textContent = s.last_response || '—';
  let known = typeof s.motor_on === 'boolean';
  $('connection-motor').textContent = known ? (s.motor_on ? 'ON' : 'OFF') : 'UNKNOWN';
  $('motor-state').textContent = known ? (s.motor_on ? 'ON' : 'OFF') : 'UNKNOWN';
  $('motor-description').textContent = known ? (s.motor_on ? 'Motor output is on' : 'Motor output is off') : 'Motor state cannot be verified';
  document.querySelector('.motor-display').classList.toggle('is-on', s.motor_on === true);
  $('action-title').textContent = known ? (s.motor_on ? 'Motor output active' : 'Ready when you are') : 'Waiting for controller';
  $('action-detail').textContent = demo ? 'Test the button in simulation mode; no physical motor will move.' : connected ? 'Use the button to command the ESP32 output.' : 'Button disabled until the ESP32 responds.';
  const button = $('motor-button'); button.disabled = busy || (!demo && !connected) || !known;
  button.classList.toggle('off-button', s.motor_on === true);
  $('motor-button-label').textContent = known ? (s.motor_on ? 'Turn motor off' : 'Turn motor on') : 'Controller unavailable';
  $('motor-button-help').textContent = demo ? 'Simulation only — no hardware command' : connected ? 'Sends command to ESP32-01' : 'Connect ESP32 to enable control';
  renderActivity(s.activity);
}
async function refresh() {
  if (busy) return;
  try { const res = await fetch('/api/status', { cache: 'no-store' }); if (!res.ok) throw Error('Server unavailable'); render(await res.json()); }
  catch (err) { render({ mode: 'esp32', connected: false, motor_on: null, activity: currentState?.activity || [], message: err.message }); }
}
$('motor-button').addEventListener('click', async () => {
  if (busy || !currentState || typeof currentState.motor_on !== 'boolean') return;
  const desired = !currentState.motor_on;
  if (!confirm(`Turn motor ${desired ? 'ON' : 'OFF'}${currentState.mode === 'demo' ? ' in DEMO MODE' : ''}?`)) return;
  busy = true; $('motor-button').disabled = true;
  try { const res = await fetch('/api/motor', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ on: desired }) }); const data = await res.json(); if (!res.ok) throw Error(data.error || 'Command failed'); }
  catch (err) { alert('Command not confirmed: ' + err.message); } finally { busy = false; await refresh(); }
});
refresh(); setInterval(refresh, 4000);
