#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "THE_SSID";
const char* password = "THE_PSWD";

WebServer server(80);
unsigned long startMillis;

int rssiToPercent(int rssi) {
  if (rssi <= -100) return 0;
  else if (rssi >= -50) return 100;
  else return 2 * (rssi + 100);
}

String getHTML() {
  String quotes[] = {
    "“Wi-Fi is the magic that makes the internet invisible yet everywhere.”",
    "“Good WiFi means less buffering and more streaming.”",
    "“Strong signal, strong connection, strong day.”",
    "“Life’s too short for weak WiFi.”",
    "“A good connection is a happy connection.”",
    "“Keep calm and boost your WiFi.”"
  };
  String quote = quotes[random(0, 6)];

  return R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>Wi-Fi Signal Dashboard</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <style>
    :root {
      --primary: #4f6ef7;
      --bg-light: #f9f9f9;
      --bg-dark: #1a1a1a;
      --text-light: #e0e0e0;
      --text-dark: #111;
      --card-light: #ffffff;
      --card-dark: #2a2a2a;
      --radius: 16px;
      --shadow: 0 8px 16px rgba(0, 0, 0, 0.08);
    }

    body {
      font-family: 'Segoe UI', sans-serif;
      margin: 0;
      padding: 0;
      background: var(--bg-light);
      color: var(--text-dark);
      transition: 0.3s;
    }

    .dark {
      background: var(--bg-dark);
      color: var(--text-light);
    }

    .container {
      max-width: 900px;
      margin: 2rem auto;
      padding: 1rem;
      display: grid;
      gap: 1rem;
      grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
    }

    .card {
      background: var(--card-light);
      padding: 1rem;
      border-radius: var(--radius);
      box-shadow: var(--shadow);
      transition: 0.3s;
    }

    .dark .card {
      background: var(--card-dark);
    }

    .progress {
      height: 20px;
      background: #e0e0e0;
      border-radius: 10px;
      overflow: hidden;
      margin-top: 8px;
    }

    .progress-fill {
      height: 100%;
      width: 0%;
      background: var(--primary);
      transition: width 0.5s;
    }

    #rssiChart {
      width: 100%;
      height: 150px;
    }

    footer {
      text-align: center;
      padding: 1rem;
      font-size: 0.9rem;
      opacity: 0.6;
    }

    button {
      padding: 8px 16px;
      background: var(--primary);
      border: none;
      color: white;
      border-radius: 8px;
      cursor: pointer;
    }

    input[type=range] {
      width: 100%;
    }
  </style>
</head>
<body>
  <header style="text-align:center; padding: 1rem;">
    <h1>Wi-Fi Signal Dashboard</h1>
  </header>
  <div class="container">
    <div class="card">
      <h2>Signal Strength</h2>
      <div><strong id="signalPercent">--%</strong> (RSSI: <span id="rssiVal">--</span> dBm)</div>
      <div class="progress"><div class="progress-fill"></div></div>
      <p id="qualityMsg">Loading...</p>
      <p id="uptime">Uptime: 0s</p>
    </div>

    <div class="card">
      <h2>Quotes</h2>
      <p>)rawliteral" + quote + R"rawliteral(</p>
    </div>

    <div class="card">
      <h2>Refresh Rate</h2>
      <label for="refreshRange" id="refreshValue">7</label> seconds
      <input type="range" id="refreshRange" min="1" max="60" value="7" />
    </div>

    <div class="card">
      <h2>Dark / Light Mode</h2>
      <button id="toggleMode">Toggle Mode</button>
    </div>

    <div class="card" style="grid-column: span 2;">
      <h2>Signal Graph</h2>
      <canvas id="rssiChart"></canvas>
    </div>
  </div>
  <footer>Developed by Hariom Sharnam</footer>

<script>
const refreshRange = document.getElementById("refreshRange");
const refreshValue = document.getElementById("refreshValue");
const toggleBtn = document.getElementById("toggleMode");
const canvas = document.getElementById("rssiChart");
const ctx = canvas.getContext("2d");
const fill = document.querySelector(".progress-fill");
const qualityMsg = document.getElementById("qualityMsg");
const uptimeEl = document.getElementById("uptime");
const percentEl = document.getElementById("signalPercent");
const rssiEl = document.getElementById("rssiVal");

let rssiData = [];
let interval = 7000;

function updateRSSI() {
  fetch("/rssi")
    .then(res => res.json())
    .then(data => {
      percentEl.textContent = data.percent + "%";
      rssiEl.textContent = data.rssi;
      fill.style.width = data.percent + "%";
      qualityMsg.textContent =
        data.percent > 80 ? "Excellent signal 👍" :
        data.percent > 60 ? "Good signal 🙂" :
        data.percent > 40 ? "Fair signal 😐" :
        data.percent > 20 ? "Poor signal 😕" :
                            "Very weak ⚠️";

      if (rssiData.length >= 50) rssiData.shift();
      rssiData.push(data.percent);
      drawGraph();
    });
}

function updateUptime() {
  fetch("/uptime")
    .then(res => res.text())
    .then(sec => {
      let mins = Math.floor(sec / 60);
      let hrs = Math.floor(mins / 60);
      let s = sec % 60;
      mins %= 60;
      uptimeEl.textContent = `Uptime: ${hrs}h ${mins}m ${s}s`;
    });
}

function drawGraph() {
  // Ensure canvas size matches visual size
  canvas.width = canvas.clientWidth;
  canvas.height = canvas.clientHeight;
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  ctx.strokeStyle = '#4f6ef7';
  ctx.lineWidth = 2;
  ctx.beginPath();

  let count = rssiData.length;
  if (count < 2) return;

  let stepX = canvas.width / (count - 1);

  for (let i = 0; i < count; i++) {
    let x = i * stepX;
    let y = canvas.height - (rssiData[i] / 100) * canvas.height;
    if (i === 0) ctx.moveTo(x, y);
    else ctx.lineTo(x, y);
  }

  ctx.stroke();
}

// Ensure canvas is sized correctly on load
window.addEventListener('load', () => {
  drawGraph();
  scheduleUpdates();
});

function scheduleUpdates() {
  updateRSSI();
  updateUptime();
  clearInterval(window._liveInterval);
  window._liveInterval = setInterval(() => {
    updateRSSI();
    updateUptime();
  }, interval);
}

refreshRange.oninput = (e) => {
  refreshValue.textContent = e.target.value;
  interval = parseInt(e.target.value) * 1000;
  scheduleUpdates();
};

toggleBtn.onclick = () => document.body.classList.toggle("dark");
</script>

</body>
</html>
)rawliteral";
}

void handleRoot() {
  server.send(200, "text/html", getHTML());
}

void handleRSSI() {
  int rssi = WiFi.RSSI();
  int percent = rssiToPercent(rssi);
  server.send(200, "application/json", "{\"rssi\":" + String(rssi) + ",\"percent\":" + String(percent) + "}");
}

void handleUptime() {
  unsigned long uptime = (millis() - startMillis) / 1000;
  server.send(200, "text/plain", String(uptime));
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

  startMillis = millis();

  server.on("/", handleRoot);
  server.on("/rssi", handleRSSI);
  server.on("/uptime", handleUptime);
  server.begin();
}

void loop() {
  server.handleClient();
}
