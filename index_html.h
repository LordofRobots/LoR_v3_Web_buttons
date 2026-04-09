#pragma once

static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html>
<head>
  <meta charset="utf-8">
  <title>Minibot Web Interface</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <style>
    :root{
      --bgTop:#F8FBFF;
      --bgBot:#F2F6FB;

      --card:#FFFFFF;
      --line:#E3E9F3;

      --fg:#0E1B2A;
      --muted:#5E6F86;

      --blue:#2FA4FF;
      --orange:#FF8A2A;
      --good:#2CCB7F;
      --warn:#FFB020;
      --bad:#E44C4C;

      --radius:16px;
      --radius2:20px;

      --shadow:0 8px 24px rgba(18,38,63,0.08);
      --shadow2:0 4px 12px rgba(18,38,63,0.06);

      --gap:12px;

      --mono: ui-monospace, SFMono-Regular, Menlo, Consolas, monospace;
      --sans: system-ui, -apple-system, Segoe UI, Roboto, Helvetica, Arial, sans-serif;
    }

    html[data-theme="dark"]{
      --bgTop:#07111F;
      --bgBot:#050C16;

      --card:#0B1526;
      --line:#1B2A44;

      --fg:#EAF0FA;
      --muted:#A7B6CE;

      --shadow:0 10px 26px rgba(0,0,0,0.40);
      --shadow2:0 6px 16px rgba(0,0,0,0.30);
    }

    html, body { height:100%; }

    body{
      margin:0;
      background: linear-gradient(180deg, var(--bgTop), var(--bgBot));
      color:var(--fg);
      font-family:var(--sans);
      overflow:hidden;
      touch-action: manipulation;
      overscroll-behavior: none;
    }

    .app{
      height:100dvh;
      max-width:1100px;
      margin:0 auto;
      padding:14px;
      box-sizing:border-box;
      display:grid;
      grid-template-rows:auto 1fr;
      gap:var(--gap);
      min-height:0;
      overflow:hidden;
    }

    .feedback{
      height:clamp(300px, 42vh, 520px);
      min-height:0;
      background:var(--card);
      border:1px solid var(--line);
      border-radius:var(--radius);
      box-shadow:var(--shadow);
      padding:12px;
      display:flex;
      flex-direction:column;
      gap:10px;
      overflow:hidden;
    }

    .topRow{
      display:flex;
      justify-content:space-between;
      align-items:flex-start;
      gap:10px;
      flex:0 0 auto;
    }

    .brand .sub{
      font-family:var(--mono);
      color:var(--muted);
      font-size:11px;
      letter-spacing:.3px;
      line-height:1.05;
    }

    .brand .title{
      font-family:var(--mono);
      font-weight:800;
      font-size:16px;
      letter-spacing:.4px;
      line-height:1.05;
      margin-top:2px;
    }

    .meta{
      display:flex;
      flex-direction:column;
      gap:6px;
      align-items:flex-end;
      font-family:var(--mono);
      font-size:11px;
      color:var(--muted);
      flex:0 0 auto;
    }

    .pill{
      display:inline-flex;
      align-items:center;
      gap:6px;
      padding:5px 9px;
      border-radius:999px;
      border:1px solid var(--line);
      background: color-mix(in srgb, var(--card) 80%, transparent);
      white-space:nowrap;
      line-height:1;
      user-select:none;
    }

    .dot{
      width:9px;
      height:9px;
      border-radius:50%;
      background:var(--bad);
      flex:0 0 auto;
    }

    .themePill{
      cursor:pointer;
      -webkit-tap-highlight-color:rgba(0,0,0,0);
      appearance:none;
      background: color-mix(in srgb, var(--card) 80%, transparent);
      border:1px solid var(--line);
      border-radius:999px;
      padding:5px 9px;
      font:inherit;
      color:inherit;
    }

    .themeIcon{
      width:12px;
      height:12px;
      border-radius:50%;
      background: linear-gradient(180deg, var(--bgTop), var(--bgBot));
      border:1px solid var(--line);
      box-shadow: inset 0 0 0 2px rgba(0,0,0,0.06);
    }

    html[data-theme="dark"] .themeIcon{
      box-shadow: inset 0 0 0 2px rgba(255,255,255,0.06);
    }

    .themePill:active{ transform:translateY(1px); }

    .themePill:focus-visible{
      outline:none;
      box-shadow:0 0 0 3px rgba(47,164,255,0.22), var(--shadow2);
    }

    .tel{
      flex:1 1 auto;
      min-height:0;
      display:grid;
      grid-template-columns: repeat(auto-fit, minmax(160px, 1fr));
      gap:10px;
      align-content:start;
      overflow:auto;
      padding-right:2px;
      -webkit-overflow-scrolling:touch;
    }

    .kv{
      background: color-mix(in srgb, var(--card) 86%, transparent);
      border:1px solid var(--line);
      border-radius:12px;
      padding:10px 10px 9px 10px;
      min-width:0;
      min-height:84px;
      overflow:hidden;
    }

    .k{
      font-family:var(--mono);
      font-size:11px;
      color:var(--muted);
      letter-spacing:.6px;
      text-transform:uppercase;
      white-space:nowrap;
      overflow:hidden;
      text-overflow:ellipsis;
      line-height:1.2;
    }

    .v{
      font-family:var(--mono);
      font-size:clamp(16px, 2.8vw, 22px);
      font-weight:800;
      margin-top:6px;
      color:var(--fg);
      white-space:nowrap;
      overflow:hidden;
      text-overflow:ellipsis;
      line-height:1.2;
    }

    .unit{
      font-size:12px;
      font-weight:800;
      color:var(--muted);
    }

    .s{
      margin-top:5px;
      font-family:var(--mono);
      font-size:10px;
      color:var(--muted);
      white-space:nowrap;
      overflow:hidden;
      text-overflow:ellipsis;
      line-height:1.2;
    }

    .kv--battery,
    .kv--status{
      position:relative;
    }

    .kv--battery::after,
    .kv--status::after{
      content:"";
      position:absolute;
      left:0;
      top:0;
      bottom:0;
      width:4px;
      border-radius:12px 0 0 12px;
      background: rgba(0,0,0,0.10);
    }

    html[data-theme="dark"] .kv--battery::after,
    html[data-theme="dark"] .kv--status::after{
      background: rgba(255,255,255,0.10);
    }

    .kv--battery.ok,
    .kv--status.ok{
      border-color: rgba(44,203,127,0.35);
      background: linear-gradient(180deg, color-mix(in srgb, var(--card) 90%, #0B2A1C), var(--card));
    }

    .kv--battery.ok::after,
    .kv--status.ok::after{
      background: rgba(44,203,127,0.85);
    }

    .kv--battery.warn,
    .kv--status.warn{
      border-color: rgba(255,176,32,0.45);
      background: linear-gradient(180deg, color-mix(in srgb, var(--card) 90%, #2B2306), var(--card));
    }

    .kv--battery.warn::after,
    .kv--status.warn::after{
      background: rgba(255,176,32,0.92);
    }

    .kv--battery.crit,
    .kv--status.crit{
      border-color: rgba(228,76,76,0.40);
      background: linear-gradient(180deg, color-mix(in srgb, var(--card) 90%, #2B0A0A), var(--card));
    }

    .kv--battery.crit::after,
    .kv--status.crit::after{
      background: rgba(228,76,76,0.92);
    }

    .inputsRow{
      display:flex;
      gap:6px;
      align-items:center;
      margin-top:8px;
      flex-wrap:wrap;
    }

    .inKey{
      display:flex;
      align-items:center;
      justify-content:center;
      gap:5px;
      padding:5px 8px;
      border-radius:999px;
      border:1px solid var(--line);
      background: color-mix(in srgb, var(--card) 88%, transparent);
      min-width:44px;
    }

    .inLbl{
      font-family:var(--mono);
      font-weight:900;
      font-size:10px;
      letter-spacing:.4px;
      color:var(--fg);
      width:12px;
      text-align:center;
      line-height:1;
      flex:0 0 auto;
    }

    .inDot{
      width:8px;
      height:8px;
      border-radius:50%;
      background: rgba(0,0,0,0.18);
      box-shadow: inset 0 0 0 2px rgba(0,0,0,0.05);
      flex:0 0 auto;
    }

    .inVal{
      font-family:var(--mono);
      font-size:10px;
      font-weight:900;
      color:var(--muted);
      line-height:1;
      flex:0 0 auto;
    }

    .inKey.active{
      border-color: rgba(47,164,255,0.55);
      background: color-mix(in srgb, var(--card) 82%, rgba(47,164,255,0.18));
    }

    .inKey.inKey--sw.active{
      border-color: rgba(255,138,42,0.60);
      background: color-mix(in srgb, var(--card) 82%, rgba(255,138,42,0.20));
    }

    .inKey.active .inLbl,
    .inKey.active .inVal{
      color:var(--fg);
    }

    html[data-theme="dark"] .inDot{
      background: rgba(255,255,255,0.14);
      box-shadow: inset 0 0 0 2px rgba(255,255,255,0.05);
    }

    .inDot.active{
      background:var(--blue);
      box-shadow:0 0 0 3px rgba(47,164,255,0.14);
    }

    .inKey--sw .inDot.active{
      background:var(--orange);
      box-shadow:0 0 0 3px rgba(255,138,42,0.14);
    }

    .vinRow{
      display:flex;
      align-items:baseline;
      justify-content:space-between;
      gap:8px;
    }

    .vinBadge{
      font-family:var(--mono);
      font-size:10px;
      font-weight:900;
      letter-spacing:.6px;
      padding:4px 8px;
      border-radius:999px;
      border:1px solid var(--line);
      color:var(--muted);
      white-space:nowrap;
    }

    #vinGraph{
      width:100%;
      height:44px;
      display:block;
      margin-top:6px;
      border-radius:10px;
      background: color-mix(in srgb, var(--card) 86%, transparent);
      border:1px solid var(--line);
    }

    #heap,
    #sys{
      white-space:normal;
      overflow:visible;
      text-overflow:clip;
      word-break:break-word;
    }

    .controls{
      min-height:0;
      height:100%;
      display:block;
    }

    .panel{
      height:100%;
      min-height:0;
      background:var(--card);
      border:1px solid var(--line);
      border-radius:var(--radius2);
      box-shadow:var(--shadow2);
      padding:12px;
      display:flex;
      justify-content:center;
      align-items:center;
      overflow:hidden;
    }

    .panel--joy{
      position:relative;
      padding-bottom:18px;
    }

    .joyWrap{
      width:100%;
      height:100%;
      min-height:0;
      display:flex;
      align-items:center;
      justify-content:center;
    }

    .joyStage{
      position:relative;
      width:min(100%, 760px);
      aspect-ratio:1 / 1;
      max-height:100%;
      display:flex;
      align-items:center;
      justify-content:center;
    }

    #joy{
      width:min(100%, 560px);
      aspect-ratio:1 / 1;
      height:auto;
      max-width:100%;
      max-height:100%;
      border-radius:50%;
      background:
        radial-gradient(circle at 50% 50%, rgba(47,164,255,0.14), transparent 60%),
        color-mix(in srgb, var(--card) 92%, transparent);
      border:2px solid var(--blue);
      box-shadow:var(--shadow);
      position:relative;
      overflow:hidden;
      touch-action:none;
      user-select:none;
      -webkit-user-select:none;
      -webkit-touch-callout:none;
    }

    #joy::before,
    #joy::after{
      content:"";
      position:absolute;
      inset:14%;
      border-radius:50%;
      border:1px dashed color-mix(in srgb, var(--fg) 14%, transparent);
      pointer-events:none;
      opacity:0.55;
    }

    #joy::after{
      inset:33%;
      opacity:0.45;
    }

    #knob{
      width:92px;
      height:92px;
      border-radius:50%;
      position:absolute;
      left:50%;
      top:50%;
      transform:translate(-50%,-50%);
      background: linear-gradient(180deg,
        color-mix(in srgb, var(--card) 96%, #FFFFFF),
        color-mix(in srgb, var(--card) 86%, rgba(47,164,255,0.25))
      );
      border:2px solid var(--orange);
      box-shadow:0 6px 16px rgba(0,0,0,0.18);
      pointer-events:none;
    }

    html[data-theme="dark"] #knob{
      box-shadow:0 8px 18px rgba(0,0,0,0.45);
    }

    .triBtn{
      position:absolute;
      width:84px;
      height:84px;
      border:none;
      background:transparent;
      cursor:pointer;
      -webkit-tap-highlight-color:rgba(0,0,0,0);
      padding:0;
      overflow:visible;
    }

    .triBtn::before{
      content:"";
      position:absolute;
      inset:0;
      background: linear-gradient(180deg,
        rgba(10,22,38,0.96),
        rgba(14,30,52,0.92)
      );
      border:2px solid var(--blue);
      box-shadow:
        0 4px 12px rgba(0,0,0,0.30),
        inset 0 0 18px rgba(47,164,255,0.08);
      z-index:0;
    }

    .triBtn span{
      position:absolute;
      z-index:2;
      font-family:var(--mono);
      font-weight:900;
      font-size:22px;
      line-height:1;
      color:#FFFFFF;
      text-shadow:
        0 2px 6px rgba(0,0,0,0.95),
        0 0 10px rgba(0,0,0,0.65);
      pointer-events:none;
    }

    .triBtn:active{
      transform:scale(0.98);
    }

    .triBtn:focus-visible{
      outline:none;
    }

    .triBtn--tl{
      top:0;
      left:0;
    }

    .triBtn--tl::before{
      clip-path:polygon(0 0, 100% 0, 0 100%);
      border-radius:16px 0 0 0;
    }

    .triBtn--tl span{
      top:14px;
      left:14px;
    }

    .triBtn--tr{
      top:0;
      right:0;
    }

    .triBtn--tr::before{
      clip-path:polygon(0 0, 100% 0, 100% 100%);
      border-radius:0 16px 0 0;
    }

    .triBtn--tr span{
      top:14px;
      right:14px;
    }

    .triBtn--bl{
      bottom:0;
      left:0;
    }

    .triBtn--bl::before{
      clip-path:polygon(0 0, 0 100%, 100% 100%);
      border-radius:0 0 0 16px;
    }

    .triBtn--bl span{
      bottom:14px;
      left:14px;
    }

    .triBtn--br{
      bottom:0;
      right:0;
    }

    .triBtn--br::before{
      clip-path:polygon(100% 0, 0 100%, 100% 100%);
      border-radius:0 0 16px 0;
    }

    .triBtn--br span{
      bottom:14px;
      right:14px;
    }

    @media (max-width: 700px){
      .topRow{
        display:grid;
        grid-template-columns:minmax(0, 1fr) 132px;
        align-items:start;
        gap:10px;
      }

      .brand{
        min-width:0;
        max-width:100%;
      }

      .meta{
        min-width:132px;
        width:132px;
        align-items:stretch;
        justify-self:end;
        gap:6px;
      }

      .pill,
      .themePill{
        width:100%;
        box-sizing:border-box;
        justify-content:flex-start;
        padding:5px 8px;
        font-size:11px;
      }

      .brand .title{
        font-size:15px;
        line-height:1.0;
      }

      .brand .sub{
        font-size:10px;
        line-height:1.0;
      }

      .tel{
        grid-template-columns:repeat(2, minmax(0, 1fr));
        gap:8px;
      }

      .app{
        padding:10px;
        gap:10px;
      }

      .panel{
        padding:10px;
      }
    }

    @media (max-width: 420px){
      .tel{
        grid-template-columns:1fr;
      }
    }

    @media (orientation: portrait) and (max-width: 720px){
      .panel{
        padding:10px;
      }

      .joyStage{
        width:100%;
      }

      #joy{
        width:min(100%, 520px);
      }

      .triBtn{
        width:72px;
        height:72px;
      }

      .triBtn span{
        font-size:20px;
      }

      #knob{
        width:84px;
        height:84px;
      }
    }

    @media (orientation: landscape) and (max-height: 650px){
      :root{ --gap:8px; }

      .app{
        grid-template-columns:minmax(300px, 420px) minmax(0, 1fr);
        grid-template-rows:1fr;
        align-items:stretch;
        padding:8px;
        gap:8px;
        height:100dvh;
        overflow:hidden;
      }

      .feedback{
        height:100%;
        min-height:0;
        padding:10px;
        gap:8px;
      }

      .tel{
        grid-template-columns:repeat(2, minmax(0, 1fr));
        gap:8px;
      }

      .kv{
        min-height:72px;
        padding:9px;
      }

      .v{
        font-size:clamp(14px, 2vw, 18px);
      }

      .s{
        font-size:9px;
      }

      .controls{
        height:100%;
        min-height:0;
      }

      .panel{
        height:100%;
        min-height:0;
        padding:8px;
      }

      .joyStage{
        width:min(100%, calc(100dvh - 32px));
        max-width:100%;
        max-height:100%;
      }

      #joy{
        width:min(100%, 440px);
        max-width:100%;
        max-height:100%;
      }

      .triBtn{
        width:64px;
        height:64px;
      }

      .triBtn span{
        font-size:18px;
      }

      #knob{
        width:68px;
        height:68px;
      }
    }

    @media (orientation: landscape) and (max-height: 430px){
      .app{
        padding:6px;
        gap:6px;
      }

      .panel{
        padding:6px;
      }

      .joyStage{
        width:min(100%, calc(100dvh - 20px));
      }

      #joy{
        width:min(100%, 360px);
      }

      .triBtn{
        width:56px;
        height:56px;
      }

      .triBtn span{
        font-size:16px;
      }

      #knob{
        width:56px;
        height:56px;
      }
    }

    .miniGraph{
      width:100%;
      height:44px;
      display:block;
      margin-top:6px;
      border-radius:10px;
      background: color-mix(in srgb, var(--card) 86%, transparent);
      border:1px solid var(--line);
    }

  </style>
</head>

<body>
<div class="app">

  <div class="feedback">
    <div class="topRow">
      <div class="brand">
        <div class="sub">LORD of ROBOTS</div>
        <div class="title">MINIBOT WEB<br>INTERFACE</div>
      </div>

      <div class="meta">
        <div class="pill" id="wsPill"><span class="dot" id="wsDot"></span><span id="wsState">WS: --</span></div>
        <div class="pill">IP: <span>10.0.0.1</span></div>
        <button class="themePill" id="themeToggle" type="button" aria-label="Toggle theme" title="Toggle light/dark">
          <span class="themeIcon" aria-hidden="true"></span>
          <span id="themeLabel">Theme: Light</span>
        </button>
      </div>
    </div>

    <div class="tel">
      <div class="kv kv--battery" id="tileVin">
        <div class="k">BATTERY</div>
        <div class="vinRow">
          <div class="v"><span id="vin">--</span> <span class="unit">V</span></div>
          <div class="vinBadge" id="vinBadge">--</div>
        </div>
        <canvas id="vinGraph" height="44" aria-label="Battery history (30s)"></canvas>
        <div class="s" id="vinState">--</div>
      </div>

      <div class="kv">
        <div class="k">VIN RAW</div>
        <div class="v" id="vinRaw">--</div>
        <div class="s">ADC</div>
      </div>

      <div class="kv kv--status" id="tileRssi">
        <div class="k">RSSI</div>
        <div class="v"><span id="rssi">--</span> <span class="unit">dBm</span></div>
        <canvas id="rssiGraph" class="miniGraph" height="44" aria-label="RSSI history (30s)"></canvas>
        <div class="s" id="rssiState">--</div>
      </div>

      <div class="kv kv--status" id="tileLag">
        <div class="k">LAG</div>
        <div class="v"><span id="lag">--</span> <span class="unit">ms</span></div>
        <canvas id="lagGraph" class="miniGraph" height="44" aria-label="Lag history (30s)"></canvas>
        <div class="s" id="lagState">--</div>
      </div>

      <div class="kv kv--status" id="tileCmdAge">
        <div class="k">CMD AGE</div>
        <div class="v"><span id="cmdAge">--</span> <span class="unit">ms</span></div>
        <canvas id="cmdAgeGraph" class="miniGraph" height="44" aria-label="Command age history (30s)"></canvas>
        <div class="s" id="cmdAgeState">since joy</div>
      </div>

      <div class="kv kv--status" id="tileHeap">
        <div class="k">HEAP</div>
        <div class="v" id="heap">--</div>
        <canvas id="heapGraph" class="miniGraph" height="44" aria-label="Heap free history (30s)"></canvas>
        <div class="s" id="heapState">free(min)</div>
      </div>

      <div class="kv kv--inputs">
        <div class="k">INPUTS</div>
        <div class="inputsRow">
          <div class="inKey" id="keyA"><div class="inLbl">A</div><div class="inDot" id="inA"></div><div class="inVal" id="inATxt">1</div></div>
          <div class="inKey" id="keyB"><div class="inLbl">B</div><div class="inDot" id="inB"></div><div class="inVal" id="inBTxt">1</div></div>
          <div class="inKey" id="keyC"><div class="inLbl">C</div><div class="inDot" id="inC"></div><div class="inVal" id="inCTxt">1</div></div>
          <div class="inKey" id="keyD"><div class="inLbl">D</div><div class="inDot" id="inD"></div><div class="inVal" id="inDTxt">1</div></div>
          <div class="inKey inKey--sw" id="keySW"><div class="inLbl">SW</div><div class="inDot" id="inSW"></div><div class="inVal" id="inSWTxt">1</div></div>
        </div>
        <div class="s" id="inputsDebug">A=1 B=1 C=1 D=1 SW=1</div>
      </div>

      <div class="kv">
        <div class="k">UPTIME</div>
        <div class="v" id="uptime">--</div>
        <div class="s">hh:mm:ss</div>
      </div>

      <div class="kv">
        <div class="k">STATIONS</div>
        <div class="v" id="stations">--</div>
        <div class="s">clients</div>
      </div>

      <div class="kv">
        <div class="k">SYSTEM</div>
        <div class="v" id="sys">--</div>
        <div class="s">cpu/flash/rst</div>
      </div>
    </div>
  </div>

  <div class="controls">
    <div class="panel panel--joy">
      <div class="joyWrap">
        <div class="joyStage">
          <button class="triBtn triBtn--tl" id="btnA" aria-label="A"><span>A</span></button>
          <button class="triBtn triBtn--tr" id="btnB" aria-label="B"><span>B</span></button>

          <div id="joy">
            <div id="knob"></div>
          </div>

          <button class="triBtn triBtn--bl" id="btnC" aria-label="C"><span>C</span></button>
          <button class="triBtn triBtn--br" id="btnD" aria-label="D"><span>D</span></button>
        </div>
      </div>
    </div>
  </div>

</div>

<script>
  const V_OK   = 7.20;
  const V_WARN = 6.80;

  const THEME_KEY = "lor_theme";
  const themeToggle = document.getElementById("themeToggle");
  const themeLabel  = document.getElementById("themeLabel");

  function safeGetTheme(){
    try { return localStorage.getItem(THEME_KEY); } catch(e){ return null; }
  }

  function safeSetTheme(v){
    try { localStorage.setItem(THEME_KEY, v); } catch(e){}
  }

  function applyTheme(theme){
    const t = (theme === "dark") ? "dark" : "light";
    if(t === "dark") document.documentElement.setAttribute("data-theme","dark");
    else document.documentElement.removeAttribute("data-theme");
    themeLabel.textContent = (t === "dark") ? "Theme: Dark" : "Theme: Light";
    safeSetTheme(t);
  }

  function currentTheme(){
    return (document.documentElement.getAttribute("data-theme") === "dark") ? "dark" : "light";
  }

  function toggleTheme(){
    applyTheme(currentTheme() === "dark" ? "light" : "dark");
  }

  applyTheme(safeGetTheme() || "light");

  themeToggle.addEventListener("click", (e)=>{ e.preventDefault(); toggleTheme(); });
  themeToggle.addEventListener("pointerup", (e)=>{ e.preventDefault(); });
  themeToggle.addEventListener("keydown", (e)=>{
    if(e.key === "Enter" || e.key === " "){
      e.preventDefault();
      toggleTheme();
    }
  });

  function getVar(name){
    return getComputedStyle(document.documentElement).getPropertyValue(name).trim();
  }

  function vinColor(v){
    if(v >= V_OK)   return getVar("--good");
    if(v >= V_WARN) return getVar("--warn");
    return getVar("--bad");
  }

  function fmtUptime(ms){
    const s = Math.floor(ms/1000);
    const h = Math.floor(s/3600);
    const m = Math.floor((s%3600)/60);
    const r = s%60;
    return `${h}h ${m}m ${r}s`;
  }

  function setInputState(dotId, keyId, txtId, rawValue, isActive){
    const dot = document.getElementById(dotId);
    const key = document.getElementById(keyId);
    const txt = document.getElementById(txtId);

    if(dot){
      if(isActive) dot.classList.add("active");
      else dot.classList.remove("active");
    }

    if(key){
      if(isActive) key.classList.add("active");
      else key.classList.remove("active");
    }

    if(txt){
      txt.textContent = String(rawValue);
    }
  }

  function setStatusTile(tileId, stateId, stateClass, stateText){
    const tile = document.getElementById(tileId);
    const s    = document.getElementById(stateId);
    if(!tile || !s) return;

    tile.classList.remove("ok","warn","crit");
    if(stateClass) tile.classList.add(stateClass);
    s.textContent = stateText;
  }

  function classifyRssi(dbm){
    if(dbm >= -60) return {cls:"ok",   txt:"GOOD"};
    if(dbm >= -70) return {cls:"warn", txt:"OK"};
    if(dbm >= -80) return {cls:"crit", txt:"WEAK"};
    return            {cls:"crit", txt:"BAD"};
  }

  function classifyLag(ms){
    if(ms <= 60)  return {cls:"ok",   txt:"GOOD"};
    if(ms <= 120) return {cls:"warn", txt:"OK"};
    if(ms <= 200) return {cls:"crit", txt:"WEAK"};
    return          {cls:"crit", txt:"BAD"};
  }

  const GRAPH_WINDOW_MS = 30000;
  let lastDraw = 0;

  function fitCanvasToCss(canvas){
    const dpr = window.devicePixelRatio || 1;
    const rect = canvas.getBoundingClientRect();
    const w = Math.max(1, Math.floor(rect.width  * dpr));
    const h = Math.max(1, Math.floor(rect.height * dpr));
    if(canvas.width !== w || canvas.height !== h){
      canvas.width = w;
      canvas.height = h;
    }
  }

  function makeGraph(canvasId, classifyFn, opts = {}){
    const canvas = document.getElementById(canvasId);
    const ctx = canvas ? canvas.getContext("2d") : null;
    const buf = [];

    function prune(now){
      const cutoff = now - GRAPH_WINDOW_MS;
      while(buf.length && buf[0].t < cutoff) buf.shift();
    }

    function push(v){
      const now = performance.now();
      if(typeof v === "number" && isFinite(v)){
        buf.push({t: now, v});
        prune(now);
      }
    }

    function colorOf(v){
      const cls = classifyFn(v);
      if(cls === "ok")   return getVar("--good");
      if(cls === "warn") return getVar("--warn");
      return getVar("--bad");
    }

    function computeRange(){
      let vmin = opts.defaultMin ?? 0;
      let vmax = opts.defaultMax ?? 1;

      if(buf.length){
        vmin = buf[0].v;
        vmax = buf[0].v;

        for(const p of buf){
          if(p.v < vmin) vmin = p.v;
          if(p.v > vmax) vmax = p.v;
        }

        const span = Math.max(0.0001, vmax - vmin);
        const pad = Math.max(opts.minPad ?? 0.05, span * (opts.padFrac ?? 0.20));
        vmin -= pad;
        vmax += pad;
      }

      if(typeof opts.clampMin === "number") vmin = Math.max(opts.clampMin, vmin);
      if(typeof opts.clampMax === "number") vmax = Math.min(opts.clampMax, vmax);

      if((vmax - vmin) < (opts.minSpan ?? 0.2)){
        const extra = (opts.minSpan ?? 0.2) / 2;
        const mid = (vmin + vmax) / 2;
        vmin = mid - extra;
        vmax = mid + extra;
      }

      return {vmin, vmax};
    }

    function draw(){
      if(!canvas || !ctx) return;

      const now = performance.now();
      prune(now);
      fitCanvasToCss(canvas);

      const w = canvas.width;
      const h = canvas.height;
      ctx.clearRect(0, 0, w, h);

      const grid = getVar("--line");
      ctx.globalAlpha = 0.6;
      ctx.strokeStyle = grid;
      ctx.lineWidth = 1;
      for(let i=1;i<=2;i++){
        const y = (h*i)/3;
        ctx.beginPath();
        ctx.moveTo(0, y);
        ctx.lineTo(w, y);
        ctx.stroke();
      }
      ctx.globalAlpha = 1.0;

      if(buf.length < 2) return;

      const {vmin, vmax} = computeRange();

      function xOf(t){
        const age = now - t;
        const frac = 1 - (age / GRAPH_WINDOW_MS);
        return Math.max(0, Math.min(w, frac * w));
      }

      function yOf(v){
        const frac = (v - vmin) / (vmax - vmin);
        return Math.max(0, Math.min(h, h - (frac * h)));
      }

      ctx.lineWidth = Math.max(2, Math.floor((window.devicePixelRatio || 1)));
      ctx.lineCap = "round";
      ctx.lineJoin = "round";

      for(let i=1;i<buf.length;i++){
        const p0 = buf[i-1];
        const p1 = buf[i];
        ctx.strokeStyle = colorOf(p1.v);
        ctx.beginPath();
        ctx.moveTo(xOf(p0.t), yOf(p0.v));
        ctx.lineTo(xOf(p1.t), yOf(p1.v));
        ctx.stroke();
      }

      const last = buf[buf.length - 1];
      ctx.fillStyle = colorOf(last.v);
      ctx.beginPath();
      ctx.arc(xOf(last.t), yOf(last.v), 3.2 * (window.devicePixelRatio || 1), 0, Math.PI * 2);
      ctx.fill();
    }

    return { push, draw, canvas };
  }

  function vinClass(v){
    if(v >= V_OK)   return "ok";
    if(v >= V_WARN) return "warn";
    return "crit";
  }

  function rssiGraphClass(v){
    if(v >= -60) return "ok";
    if(v >= -70) return "warn";
    return "crit";
  }

  function lagGraphClass(v){
    if(v <= 60)  return "ok";
    if(v <= 120) return "warn";
    return "crit";
  }

  function cmdAgeGraphClass(v){
    if(v <= 150) return "ok";
    if(v <= 400) return "warn";
    return "crit";
  }

  function heapGraphClass(v){
    if(v >= 80000) return "ok";
    if(v >= 50000) return "warn";
    return "crit";
  }

  const vinGraph = makeGraph("vinGraph", vinClass, {
    defaultMin: 6.4,
    defaultMax: 8.4,
    clampMin: 0,
    clampMax: 20,
    minPad: 0.06,
    minSpan: 0.2
  });

  const rssiGraph = makeGraph("rssiGraph", rssiGraphClass, {
    defaultMin: -90,
    defaultMax: -40,
    clampMin: -100,
    clampMax: 0,
    minPad: 2,
    minSpan: 8
  });

  const lagGraph = makeGraph("lagGraph", lagGraphClass, {
    defaultMin: 0,
    defaultMax: 200,
    clampMin: 0,
    minPad: 5,
    minSpan: 20
  });

  const cmdAgeGraph = makeGraph("cmdAgeGraph", cmdAgeGraphClass, {
    defaultMin: 0,
    defaultMax: 500,
    clampMin: 0,
    minPad: 10,
    minSpan: 40
  });

  const heapGraph = makeGraph("heapGraph", heapGraphClass, {
    defaultMin: 30000,
    defaultMax: 120000,
    clampMin: 0,
    minPad: 2000,
    minSpan: 10000
  });

  (function graphLoop(){
    const now = performance.now();
    if(now - lastDraw >= 50){
      lastDraw = now;
      vinGraph.draw();
      rssiGraph.draw();
      lagGraph.draw();
      cmdAgeGraph.draw();
      heapGraph.draw();
    }
    requestAnimationFrame(graphLoop);
  })();

  window.addEventListener("resize", ()=>{
    if(vinGraph.canvas)    fitCanvasToCss(vinGraph.canvas);
    if(rssiGraph.canvas)   fitCanvasToCss(rssiGraph.canvas);
    if(lagGraph.canvas)    fitCanvasToCss(lagGraph.canvas);
    if(cmdAgeGraph.canvas) fitCanvasToCss(cmdAgeGraph.canvas);
    if(heapGraph.canvas)   fitCanvasToCss(heapGraph.canvas);
  });

  let ws;
  const wsDot   = document.getElementById("wsDot");
  const wsPill  = document.getElementById("wsPill");
  const wsState = document.getElementById("wsState");

  let lastTelMs = 0;
  let lagEmaMs = null;
  const LAG_EMA_ALPHA = 0.20;

  function setWsUI(connected){
    wsDot.style.background = connected ? getVar('--good') : getVar('--bad');
    wsState.textContent = connected ? "WS: connected" : "WS: disconnected";
    wsPill.style.borderColor = connected ? "rgba(44,203,127,0.35)" : "rgba(228,76,76,0.35)";
    wsPill.style.background  = connected ? "color-mix(in srgb, var(--card) 92%, #DFF7EA)" : "color-mix(in srgb, var(--card) 92%, #F9DFDF)";
  }

  function wsSend(obj){
    if(ws && ws.readyState === WebSocket.OPEN){
      ws.send(JSON.stringify(obj));
    }
  }

  function wsConnect(){
    setWsUI(false);
    ws = new WebSocket(`ws://${location.host}/ws`);

    ws.onopen  = () => setWsUI(true);
    ws.onclose = () => { setWsUI(false); setTimeout(wsConnect, 400); };
    ws.onerror = () => { setWsUI(false); };

    ws.onmessage = (e) => {
      try{
        const m = JSON.parse(e.data);
        if(m.type !== "tel") return;

        const now = performance.now();

        try{
          if(lastTelMs > 0){
            const dt = now - lastTelMs;
            lagEmaMs = (lagEmaMs === null) ? dt : (LAG_EMA_ALPHA * dt + (1 - LAG_EMA_ALPHA) * lagEmaMs);
            const lagMs = Math.round(lagEmaMs);
            document.getElementById("lag").textContent = `${lagMs}`;
            lagGraph.push(lagMs);
            const lagClass = classifyLag(lagMs);
            setStatusTile("tileLag", "lagState", lagClass.cls, lagClass.txt);
          } else {
            document.getElementById("lag").textContent = `--`;
            setStatusTile("tileLag", "lagState", "", "--");
          }
          lastTelMs = now;
        } catch(err){ console.error("lag update failed", err); }

        try{
          const vin = (typeof m.vin === "number") ? m.vin : null;
          document.getElementById("vin").textContent    = (vin === null) ? "--" : vin.toFixed(2);
          document.getElementById("vinRaw").textContent = (m.vin_raw ?? "--");
          if(vin !== null) vinGraph.push(vin);

          const tileVin  = document.getElementById("tileVin");
          const vinState = document.getElementById("vinState");
          const badge    = document.getElementById("vinBadge");

          tileVin.classList.remove("ok","warn","crit");

          if(vin === null){
            vinState.textContent = "--";
            badge.textContent = "--";
            badge.style.borderColor = getVar("--line");
            badge.style.color = getVar("--muted");
          } else {
            const cls = vinClass(vin);
            tileVin.classList.add(cls);
            vinState.textContent = (cls === "ok") ? "OK" : (cls === "warn") ? "LOW" : "CRIT";
            badge.textContent = vinState.textContent;
            const c = vinColor(vin);
            badge.style.borderColor = c;
            badge.style.color = c;
          }
        } catch(err){ console.error("vin update failed", err); }

        try{
          const rssi = (typeof m.rssi_dbm === "number") ? m.rssi_dbm : null;
          document.getElementById("rssi").textContent = (rssi === null) ? "--" : `${rssi}`;
          if(rssi !== null) rssiGraph.push(rssi);

          if(rssi === null){
            setStatusTile("tileRssi", "rssiState", "", "--");
          } else {
            const r = classifyRssi(rssi);
            setStatusTile("tileRssi", "rssiState", r.cls, r.txt);
          }
        } catch(err){ console.error("rssi update failed", err); }

        try{
          const rawA  = (m.btnA ?? 1);
          const rawB  = (m.btnB ?? 1);
          const rawC  = (m.btnC ?? 1);
          const rawD  = (m.btnD ?? 1);
          const rawSW = (m.sw   ?? 1);

          const A  = rawA  === 0;
          const B  = rawB  === 0;
          const C  = rawC  === 0;
          const D  = rawD  === 0;
          const SW = rawSW === 0;

          setInputState("inA",  "keyA",  "inATxt",  rawA,  A);
          setInputState("inB",  "keyB",  "inBTxt",  rawB,  B);
          setInputState("inC",  "keyC",  "inCTxt",  rawC,  C);
          setInputState("inD",  "keyD",  "inDTxt",  rawD,  D);
          setInputState("inSW", "keySW", "inSWTxt", rawSW, SW);

          const dbg = document.getElementById("inputsDebug");
          if(dbg){
            dbg.textContent = `A=${rawA} B=${rawB} C=${rawC} D=${rawD} SW=${rawSW}`;
          }
        } catch(err){ console.error("inputs update failed", err); }

        try{
          document.getElementById("uptime").textContent   = fmtUptime(m.uptime_ms ?? 0);
          document.getElementById("stations").textContent = (m.stations ?? "--");
        } catch(err){ console.error("uptime/stations update failed", err); }

        try{
          const cmdAge = (typeof m.cmd_age_ms === "number") ? m.cmd_age_ms : null;
          document.getElementById("cmdAge").textContent = (cmdAge === null) ? "--" : `${cmdAge}`;

          if(cmdAge !== null){
            cmdAgeGraph.push(cmdAge);
            const c = cmdAgeGraphClass(cmdAge);
            setStatusTile("tileCmdAge", "cmdAgeState", c, (c === "ok") ? "FRESH" : (c === "warn") ? "AGING" : "STALE");
          } else {
            setStatusTile("tileCmdAge", "cmdAgeState", "", "since joy");
          }
        } catch(err){ console.error("cmd age update failed", err); }

        try{
          const heapFree = (typeof m.heap_free === "number") ? m.heap_free : null;
          const heapMin  = (typeof m.heap_min  === "number") ? m.heap_min  : null;

          document.getElementById("heap").textContent =
            `${heapFree === null ? "--" : heapFree} (${heapMin === null ? "--" : heapMin})`;

          if(heapFree !== null){
            heapGraph.push(heapFree);
            const h = heapGraphClass(heapFree);
            setStatusTile("tileHeap", "heapState", h, (h === "ok") ? "GOOD" : (h === "warn") ? "LOW" : "CRIT");
          } else {
            setStatusTile("tileHeap", "heapState", "", "free(min)");
          }
        } catch(err){ console.error("heap update failed", err); }

        try{
          const cpu   = (m.cpu_mhz ?? "--");
          const flash = (m.flash_mb ?? "--");
          const rst   = (m.reset ?? "--");
          document.getElementById("sys").textContent = `${cpu}/${flash}/${rst}`;
        } catch(err){ console.error("sys update failed", err); }
      }catch(err){}
    };
  }

  wsConnect();

  document.getElementById("btnA").addEventListener("pointerdown", ()=>wsSend({type:"fn", id:"A"}));
  document.getElementById("btnB").addEventListener("pointerdown", ()=>wsSend({type:"fn", id:"B"}));
  document.getElementById("btnC").addEventListener("pointerdown", ()=>wsSend({type:"fn", id:"C"}));
  document.getElementById("btnD").addEventListener("pointerdown", ()=>wsSend({type:"fn", id:"D"}));

  const joy  = document.getElementById("joy");
  const knob = document.getElementById("knob");

  let active = false;
  let center = {x:0,y:0};
  let radius = 0;

  let joyX = 0;
  let joyY = 0;

  let sendTimer = null;
  const sendHz = 30;
  const sendInterval = 1000 / sendHz;

  function updateGeometry(){
    const r = joy.getBoundingClientRect();
    center.x = r.left + r.width/2;
    center.y = r.top  + r.height/2;
    radius = (r.width/2) - 58;
    if(radius < 30) radius = 30;
  }

  function clamp(v,lo,hi){
    return Math.max(lo, Math.min(hi, v));
  }

  function setKnob(dx,dy){
    knob.style.transform = `translate(${dx}px, ${dy}px) translate(-50%,-50%)`;
  }

  function startJoySender(){
    if(sendTimer) return;
    sendTimer = setInterval(() => {
      if(active){
        wsSend({ type:"joy", x: joyX, y: joyY });
      } else {
        wsSend({ type:"joy", x: 0, y: 0 });
      }
    }, sendInterval);
  }

  function handleMove(clientX, clientY){
    const dx = clientX - center.x;
    const dy = clientY - center.y;

    const mag = Math.hypot(dx, dy);
    let px = dx, py = dy;
    if(mag > radius){
      const s = radius / mag;
      px *= s;
      py *= s;
    }

    setKnob(px, py);

    joyX = clamp(px / radius, -1, 1);
    joyY = clamp((-py) / radius, -1, 1);
  }

  function stop(){
    active = false;
    joyX = 0;
    joyY = 0;
    setKnob(0,0);
    wsSend({type:"joy", x:0, y:0});
    document.body.style.overflow = "";
  }

  function startAt(clientX, clientY){
    updateGeometry();
    active = true;
    document.body.style.overflow = "hidden";
    handleMove(clientX, clientY);
    wsSend({type:"joy", x:joyX, y:joyY});
    startJoySender();
  }

  joy.addEventListener("pointerdown", (e)=>{
    e.preventDefault();
    joy.setPointerCapture?.(e.pointerId);
    startAt(e.clientX, e.clientY);
  }, {passive:false});

  joy.addEventListener("pointermove", (e)=>{
    if(!active) return;
    e.preventDefault();
    handleMove(e.clientX, e.clientY);
  }, {passive:false});

  joy.addEventListener("pointerup", (e)=>{ e.preventDefault(); stop(); }, {passive:false});
  joy.addEventListener("pointercancel", (e)=>{ e.preventDefault(); stop(); }, {passive:false});
  joy.addEventListener("lostpointercapture", ()=>{ if(active) stop(); });

  joy.addEventListener("touchstart", (e)=>{
    e.preventDefault();
    const t = e.changedTouches[0];
    startAt(t.clientX, t.clientY);
  }, {passive:false});

  joy.addEventListener("touchmove", (e)=>{
    if(!active) return;
    e.preventDefault();
    const t = e.changedTouches[0];
    handleMove(t.clientX, t.clientY);
  }, {passive:false});

  joy.addEventListener("touchend", (e)=>{ e.preventDefault(); stop(); }, {passive:false});
  joy.addEventListener("touchcancel", (e)=>{ e.preventDefault(); stop(); }, {passive:false});

  window.addEventListener("resize", updateGeometry);
  updateGeometry();
  startJoySender();
</script>
</body>
</html>
)rawliteral";