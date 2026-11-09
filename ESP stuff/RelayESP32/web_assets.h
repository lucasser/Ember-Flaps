#pragma once
// Auto-embedded webpage. Edit source web files and rerun build_assets.py to regenerate.
const char HTML_DATA[] PROGMEM = R"WHTML2026(<!doctype html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta name="theme-color" content="#1a2d26">
  <title>relay — Motor control</title>
  <link rel="stylesheet" href="/style.css">
</head>
<body>
<header class="site-header">
  <div class="header-inside">
    <div class="brand-section">
      <div class="brand-line"><div class="brand-icon" aria-hidden="true">⏻</div><span class="brand">relay</span><span class="slash">/</span><span class="console">Device console</span></div>
      <div class="crumb">Workspace <span>/</span> <strong>ESP32-01</strong></div>
    </div>
    <div class="header-right"><span class="overview">Overview</span><span class="header-divider"></span><span class="connection-label"><span class="status-dot" id="header-dot"></span><span id="header-connection">Checking connection</span></span></div>
  </div>
</header>
<main class="container">
  <div class="page-title"><div><h1>Motor control</h1><p>Your connection and motor power. One clear view.</p></div><span class="last-updated"><span class="status-dot" id="updated-dot"></span><span id="updated">Checking status…</span></span></div>
  <div class="demo-banner" id="demo-banner" hidden>DEMO MODE — Buttons simulate motor commands. No ESP32 is connected.</div>
  <div class="main-grid">
    <section class="card motor-card">
      <div class="card-header"><h2><span class="mini-icon">☷</span> Motor output</h2><span class="monospace subdued">MOTOR / 01</span></div>
      <div class="motor-display"><div class="motor-copy"><div class="eyebrow"><span class="little-dot"></span> SYSTEM STATE</div><div class="big-status" id="motor-state">UNKNOWN</div><p id="motor-description">Checking motor state</p></div><div class="power-art" aria-hidden="true"><div class="power-ring"><div class="power-center"><span>⏻</span></div></div></div></div>
      <div class="motor-controls"><div class="motor-instructions"><h3 id="action-title">Checking device</h3><p id="action-detail">Connecting to controller…</p></div><div class="motor-button-area"><button id="motor-button" class="motor-button" disabled><span>⏻</span><span id="motor-button-label">Checking status</span></button><small id="motor-button-help">Controls ESP32-01 when connected</small></div></div>
    </section>
    <section class="card connection-card">
      <div class="card-header"><h2>ESP32 connection</h2><span class="chip-icon" aria-hidden="true">▦</span></div>
      <div class="connection-body"><div class="connection-head"><span class="wifi-icon">⌁</span><span id="connection-state">Checking…</span></div><p id="connection-description">Waiting for device status.</p><div class="info-lines"><div><span>Device</span><strong class="monospace">ESP32-01</strong></div><div><span>Last response</span><strong id="last-response">—</strong></div><div><span>Motor output</span><strong class="monospace" id="connection-motor">UNKNOWN</strong></div></div></div>
      <div class="connection-note"><span>ⓘ</span><div><strong>Connected doesn’t mean running</strong><p>The ESP32 connection stays active when motor power is off.</p></div></div>
    </section>
  </div>
  <section class="card activity-card"><div class="activity-heading"><h2><span class="mini-icon">◴</span> Recent activity</h2><span>This session · Today</span></div><div id="activity-list" class="activity-list"><div class="empty-activity">No activity recorded yet.</div></div></section>
  <p class="footer-caution">Prototype interface. ON/OFF reflects ESP32 GPIO command state, not proof of motor motion or fire protection. Not a safety-critical control system.</p>
</main>
<script src="/script.js"></script>
</body>
</html>
)WHTML2026";

const char CSS_DATA[] PROGMEM = R"WCSS2026(:root{font-family:Inter,ui-sans-serif,-apple-system,BlinkMacSystemFont,"Segoe UI",Arial,sans-serif;color:#1b2b25;background:#f4f5f4;font-synthesis:none}*{box-sizing:border-box}body{margin:0;font-size:15px}button{font:inherit}.site-header{background:#fff;border-bottom:1px solid #e0e6e2}.header-inside{max-width:1400px;margin:auto;min-height:79px;padding:13px 24px;display:flex;align-items:center;justify-content:space-between;gap:24px}.brand-line{display:flex;align-items:center;gap:15px}.brand-icon{display:grid;place-items:center;width:44px;height:44px;border-radius:10px;background:#1b3028;color:white;font-size:30px;font-weight:300}.brand{font-size:26px;letter-spacing:-1px}.slash{font-size:24px;color:#9aa6a0}.console{font-size:16px;color:#6d7a73}.crumb{font-size:13px;color:#69776f;padding-top:1px}.crumb span{padding:0 9px;color:#9aa6a0}.crumb strong{color:#24362e;font-weight:500}.header-right{display:flex;gap:27px;align-items:center;color:#6b7972}.overview{color:#0f704d;font-weight:700}.header-divider{height:27px;width:1px;background:#dce4df}.connection-label{display:flex;align-items:center;gap:10px}.status-dot{width:8px;height:8px;flex:none;border-radius:50%;background:#99a6a0;display:inline-block}.status-dot.connected{background:#149366}.status-dot.disconnected{background:#b27a41}.container{max-width:1400px;padding:15px 24px 28px;margin:0 auto}.page-title{display:flex;justify-content:space-between;align-items:end;margin-bottom:30px;gap:20px}.page-title h1{font-size:40px;font-weight:750;letter-spacing:-1.7px;margin:0 0 5px}.page-title p{margin:0;color:#728078;font-size:17px}.last-updated{display:flex;align-items:center;gap:10px;font-size:13px;color:#728078;padding-bottom:6px;white-space:nowrap}.demo-banner{background:#fff8ea;border:1px solid #efdcad;color:#816022;border-radius:9px;padding:12px 16px;margin-bottom:17px;font-weight:650;font-size:13px}.demo-banner[hidden]{display:none}.main-grid{display:grid;grid-template-columns:minmax(0,1.78fr) minmax(340px,1fr);gap:26px}.card{background:#fff;border:1px solid #dbe4df;border-radius:18px;overflow:hidden}.motor-card{padding:30px;min-height:515px}.card-header{display:flex;align-items:center;justify-content:space-between;gap:12px}.card h2{font-size:18px;line-height:1.4;margin:0;font-weight:700;letter-spacing:-.2px}.mini-icon{font-size:21px;color:#5a7063;vertical-align:-1px;margin-right:8px}.monospace{font-family:ui-monospace,SFMono-Regular,Menlo,monospace;font-size:12px}.subdued{color:#76867d}.motor-display{background:#f5f7f5;border:1px solid #dce6e0;border-radius:10px;min-height:260px;margin-top:28px;display:flex;align-items:center;justify-content:space-between;padding:30px;gap:15px}.eyebrow{font:12px ui-monospace,SFMono-Regular,monospace;color:#65776b;display:flex;align-items:center;gap:9px}.little-dot{width:9px;height:9px;background:#5e7468;border-radius:50%}.big-status{font-size:84px;letter-spacing:-4px;font-weight:350;line-height:1.3}.motor-copy p{font-size:18px;color:#6b7b71;margin:2px 0}.power-art{flex:none;width:188px;height:188px;border:1px solid #dce5de;border-radius:50%;display:grid;place-items:center;position:relative}.power-art:before{content:"";position:absolute;width:159px;height:159px;border:1px solid #dce5de;border-radius:50%}.power-art:after{content:"";position:absolute;top:3px;width:9px;height:9px;background:#62776c;border-radius:50%}.power-center{width:134px;height:134px;display:grid;place-items:center;background:white;border:1px solid #dce5de;border-radius:50%;font-size:59px;color:#536e5e}.motor-display.is-on .power-center{color:#128052}.motor-controls{display:flex;align-items:center;justify-content:space-between;gap:18px;margin-top:37px}.motor-instructions{max-width:360px}.motor-instructions h3{font-size:17px;margin:0 0 9px}.motor-instructions p{color:#728078;line-height:1.6;margin:0;font-size:14px}.motor-button-area{width:42%;min-width:230px;text-align:center}.motor-button{width:100%;min-height:63px;border:0;border-radius:9px;color:#fff;background:#137650;font-size:18px;font-weight:650;display:flex;gap:13px;align-items:center;justify-content:center;cursor:pointer;transition:background .15s,transform .15s}.motor-button span:first-child{font-size:27px}.motor-button:hover:not(:disabled){background:#0b5d40}.motor-button:active:not(:disabled){transform:scale(.99)}.motor-button.off-button{background:#273c33}.motor-button:disabled{opacity:.53;cursor:not-allowed}.motor-button-area small{font-size:12px;color:#748179;display:block;margin-top:11px}.connection-card{display:flex;flex-direction:column;min-height:515px}.connection-card .card-header{padding:30px 30px 0}.chip-icon{font-size:25px;color:#728179}.connection-body{padding:56px 30px 25px;flex:1}.connection-head{display:flex;gap:15px;align-items:center;font-size:36px;color:#157850;font-weight:350;letter-spacing:-1px}.connection-head.not-connected{color:#91734a}.wifi-icon{font-size:30px;background:#e6f4ec;color:#117e54;width:48px;height:48px;border-radius:50%;display:grid;place-items:center}.connection-body>p{color:#718177;line-height:1.6;margin:10px 0 30px}.info-lines{border-top:1px solid #dce4df;padding-top:15px}.info-lines>div{display:flex;align-items:center;justify-content:space-between;gap:14px;padding:15px 0;color:#6d7b73}.info-lines strong{color:#293a30;font-size:13px;font-weight:500}.connection-note{display:flex;gap:15px;background:#e9f5ed;padding:22px 30px;color:#0c7b54;border-top:1px solid #dce9df;min-height:120px}.connection-note>span{font-size:22px}.connection-note strong{font-size:13px}.connection-note p{margin:7px 0 0;color:#63766b;font-size:13px;line-height:1.5}.activity-card{margin-top:30px;padding:22px 30px}.activity-heading{display:flex;align-items:center;justify-content:space-between;gap:15px;margin-bottom:17px}.activity-heading>span{font-size:13px;color:#76847b}.activity-row{display:grid;grid-template-columns:130px 1fr 1.85fr 100px;align-items:center;gap:16px;border-top:1px solid #e0e7e2;min-height:55px;font-size:13px}.activity-time{font-family:ui-monospace,SFMono-Regular,Menlo,monospace;color:#78877d}.activity-name{font-weight:550}.activity-detail{color:#748279}.activity-confirm{color:#147c52;text-align:right}.empty-activity{color:#75847a;border-top:1px solid #e1e7e3;padding:22px 0;font-size:14px}.footer-caution{font-size:12px;color:#75847a;line-height:1.5;margin:17px 0}@media(max-width:920px){.main-grid{grid-template-columns:1fr}.motor-card,.connection-card{min-height:auto}.connection-body{padding-top:30px}.motor-button-area{width:45%}}@media(max-width:650px){.header-inside{padding:12px 16px}.header-right{display:none}.container{padding:20px 14px}.page-title{align-items:start;flex-direction:column;margin-bottom:20px}.page-title h1{font-size:33px}.page-title p{font-size:14px}.last-updated{font-size:12px}.motor-card{padding:18px}.motor-display{padding:18px;min-height:205px}.big-status{font-size:55px}.power-art{width:105px;height:105px}.power-art:before{width:90px;height:90px}.power-center{width:76px;height:76px;font-size:38px}.motor-copy p{font-size:14px}.motor-controls{flex-direction:column;align-items:stretch;margin-top:25px}.motor-button-area{min-width:0;width:100%}.connection-card .card-header{padding:20px 20px 0}.connection-body{padding:25px 20px}.connection-head{font-size:28px}.connection-note{padding:18px 20px}.activity-card{padding:18px 16px}.activity-row{grid-template-columns:70px 1fr auto;gap:7px;padding:12px 0}.activity-detail{grid-column:2 / -1;grid-row:2}.activity-confirm{grid-column:3;grid-row:1}.activity-time{font-size:11px}.activity-heading>span{display:none}}
)WCSS2026";

const char JS_DATA[] PROGMEM = R"WJS2026(const $ = (id) => document.getElementById(id);
let currentState = null;
let busy = false;
function setDot(id, connected) { $(id).classList.toggle('connected', connected); $(id).classList.toggle('disconnected', !connected); }
function renderActivity(items) {
  const list = $('activity-list'); list.replaceChildren();
  if (!items || !items.length) { const row=document.createElement('div');row.className='empty-activity';row.textContent='No activity recorded yet.';list.append(row);return; }
  for (const item of items) {
    const row=document.createElement('div');row.className='activity-row';
    const fields=[['activity-time',item.time],['activity-name',item.label],['activity-detail',item.detail],['activity-confirm','✓ Reported']];
    for (const [className,value] of fields){const span=document.createElement('span');span.className=className;span.textContent=value;row.append(span);}list.append(row);
  }
}
function render(s) {
  currentState=s;
  const demo=s.mode==='demo';const connected=s.connected===true;
  $('demo-banner').hidden=!demo;
  $('header-connection').textContent=demo?'Demo mode':connected?'1 device connected':'Device disconnected';
  setDot('header-dot',connected);setDot('updated-dot',connected);
  $('updated').textContent=connected?'Last response received':demo?'Simulated locally':'Controller unavailable';
  $('connection-state').textContent=demo?'Demo mode':connected?'Connected':'Disconnected';
  document.querySelector('.connection-head').classList.toggle('not-connected',!connected);
  $('connection-description').textContent=demo?'This is a simulated motor state. No ESP32 has been detected.':connected?'Your controller responded to the latest status request.':'No response from ESP32. Check power and Wi-Fi.';
  $('last-response').textContent=s.last_response||'—';
  let known=typeof s.motor_on==='boolean';
  $('connection-motor').textContent=known?(s.motor_on?'ON':'OFF'):'UNKNOWN';
  $('motor-state').textContent=known?(s.motor_on?'ON':'OFF'):'UNKNOWN';
  $('motor-description').textContent=known?(s.motor_on?'Motor output is on':'Motor output is off'):'Motor state cannot be verified';
  document.querySelector('.motor-display').classList.toggle('is-on',s.motor_on===true);
  $('action-title').textContent=known?(s.motor_on?'Motor output active':'Ready when you are'):'Waiting for controller';
  $('action-detail').textContent=demo?'Test the button in simulation mode; no physical motor will move.':connected?'Use the button to command the ESP32 output.':'Button disabled until the ESP32 responds.';
  const button=$('motor-button');button.disabled=busy||(!demo&&!connected)||!known;
  button.classList.toggle('off-button',s.motor_on===true);
  $('motor-button-label').textContent=known?(s.motor_on?'Turn motor off':'Turn motor on'):'Controller unavailable';
  $('motor-button-help').textContent=demo?'Simulation only — no hardware command':connected?'Sends command to ESP32-01':'Connect ESP32 to enable control';
  renderActivity(s.activity);
}
async function refresh() {
  if (busy) return;
  try {const res=await fetch('/api/status',{cache:'no-store'});if(!res.ok)throw Error('Server unavailable');render(await res.json());}
  catch(err){render({mode:'esp32',connected:false,motor_on:null,activity:currentState?.activity||[],message:err.message});}
}
$('motor-button').addEventListener('click',async()=>{
  if(busy||!currentState||typeof currentState.motor_on!=='boolean')return;
  const desired=!currentState.motor_on;
  if (!confirm(`Turn motor ${desired?'ON':'OFF'}${currentState.mode==='demo'?' in DEMO MODE':''}?`)) return;
  busy=true;$('motor-button').disabled=true;
  try {const res=await fetch('/api/motor',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({on:desired})});const data=await res.json();if(!res.ok)throw Error(data.error||'Command failed');}
  catch(err){alert('Command not confirmed: '+err.message);}finally{busy=false;await refresh();}
});
refresh();setInterval(refresh,4000);
)WJS2026";

