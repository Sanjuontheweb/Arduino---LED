# Svelte Arduino Controller

This document contains a minimal Svelte + Vite project that demonstrates two ways to control Arduino chips from a web UI:

1. **Direct browser → Arduino (Web Serial API)** — *no server required*, works in Chromium-based browsers with `navigator.serial` support (Chrome, Edge). Best for local dev and direct USB connections.
2. **Browser → Backend → Arduino (Node + serialport)** — if you need cross-browser support, remote access, or want to run on a server.

Included files below: `package.json`, `index.html`, `src/main.js`, `src/App.svelte`, `arduino/sketch.ino`, and `server/server.js` (Node example).

---

## Quick start (Web Serial version)

1. Ensure your browser supports Web Serial (Chrome/Edge on desktop).
2. Create a new Vite Svelte project or copy the files below into a Vite Svelte scaffold.
3. Install deps and run:

```bash
npm install
npm run dev
```

4. Open the app in your browser. Plug in Arduino via USB and use the `Connect` button.

> **Security/Permissions**: the browser will prompt for permission to access the serial device.

---

## package.json

```json
{
  "name": "svelte-arduino-controller",
  "version": "0.1.0",
  "private": true,
  "type": "module",
  "scripts": {
    "dev": "vite",
    "build": "vite build",
    "preview": "vite preview"
  },
  "dependencies": {
    "svelte": "^4.0.0"
  },
  "devDependencies": {
    "@sveltejs/vite-plugin-svelte": "^2.0.0",
    "vite": "^5.0.0"
  }
}
```

---

## index.html

```html
<!doctype html>
<html>
  <head>
    <meta charset="utf-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>Svelte Arduino Controller</title>
  </head>
  <body>
    <div id="app"></div>
    <script type="module" src="/src/main.js"></script>
  </body>
</html>
```

---

## src/main.js

```js
import App from './App.svelte';

const app = new App({
  target: document.getElementById('app')
});

export default app;
```

---

## src/App.svelte (Web Serial UI)

```svelte
<script>
  import { onMount } from 'svelte';
  let port = null;
  let writer = null;
  let reader = null;
  let log = '';
  let connected = false;
  let incoming = '';
  let ledState = false;

  async function connect() {
    try {
      // Request a port
      port = await navigator.serial.requestPort();
      // Open at the Arduino default baud
      await port.open({ baudRate: 9600 });
      connected = true;
      log += 'Connected to port.\n';

      // Setup writer
      writer = port.writable.getWriter();

      // Setup reader loop
      const textDecoder = new TextDecoderStream();
      const readableStreamClosed = port.readable.pipeTo(textDecoder.writable);
      reader = textDecoder.readable.getReader();

      readLoop();
    } catch (err) {
      log += `Connect error: ${err.message}\n`;
    }
  }

  async function readLoop() {
    try {
      while (true) {
        const { value, done } = await reader.read();
        if (done) break;
        if (value) {
          incoming += value;
          log += `← ${value}`;
        }
      }
    } catch (err) {
      log += `Read error: ${err.message}\n`;
    }
  }

  async function disconnect() {
    try {
      if (reader) {
        await reader.cancel();
        reader.releaseLock();
        reader = null;
      }
      if (writer) {
        writer.releaseLock();
        writer = null;
      }
      if (port) {
        await port.close();
        port = null;
      }
      connected = false;
      log += 'Disconnected.\n';
    } catch (err) {
      log += `Disconnect error: ${err.message}\n`;
    }
  }

  async function send(text) {
    if (!writer) {
      log += 'Not connected or writer missing.\n';
      return;
    }
    try {
      const data = new TextEncoder().encode(text + '\n');
      await writer.write(data);
      log += `→ ${text}\n`;
    } catch (err) {
      log += `Write error: ${err.message}\n`;
    }
  }

  function toggleLED() {
    ledState = !ledState;
    send(ledState ? 'LED_ON' : 'LED_OFF');
  }

  let customCmd = '';
</script>

<main style="font-family: system-ui, sans-serif; padding: 1rem;">
  <h1>Svelte → Arduino (Web Serial)</h1>

  {#if 'serial' in navigator}
    <div style="margin-bottom: 1rem;">
      {#if !connected}
        <button on:click={connect}>Connect</button>
      {:else}
        <button on:click={disconnect}>Disconnect</button>
      {/if}
    </div>

    <div style="margin-bottom: 1rem;">
      <button on:click={toggleLED} disabled={!connected}>{ledState ? 'Turn LED off' : 'Turn LED on'}</button>
    </div>

    <div style="margin-bottom: 1rem;">
      <input bind:value={customCmd} placeholder="Custom command (e.g. STATUS)" />
      <button on:click={() => send(customCmd)} disabled={!connected}>Send</button>
    </div>

    <h2>Log</h2>
    <textarea readonly rows="10" style="width:100%;">{log}</textarea>

    <h2>Raw incoming</h2>
    <pre style="background:#f6f6f6; padding:8px;">{incoming}</pre>
  {:else}
    <p>Your browser does not support the Web Serial API. Use Chrome/Edge or try the Node backend approach.</p>
  {/if}

  <hr />
  <p style="font-size:0.9rem; color:#555">Notes: This app sends newline-terminated commands. Keep the Arduino sketch set to the same baud (9600) and parse commands there.</p>
</main>
```

---

## Arduino sketch (upload to your board)

```cpp
// sketch.ino

const int LED_PIN = 13; // builtin LED

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  Serial.println("ARDUINO_READY");
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "LED_ON") {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("OK:LED_ON");
    } else if (cmd == "LED_OFF") {
      digitalWrite(LED_PIN, LOW);
      Serial.println("OK:LED_OFF");
    } else if (cmd == "STATUS") {
      int state = digitalRead(LED_PIN);
      Serial.print("STATUS:");
      Serial.println(state);
    } else {
      Serial.print("UNKNOWN:");
      Serial.println(cmd);
    }
  }
}
```

---

## Optional: Node backend (express + serialport)

> Use this if you prefer the browser to talk to a local/remote server which then controls the Arduino. Install `npm i express serialport`.

```js
// server/server.js
import express from 'express';
import { SerialPort } from 'serialport';

const app = express();
app.use(express.json());

// Adjust this path to your Arduino serial port (COMx on Windows, /dev/ttyUSBx on Linux)
const ARDUINO_PATH = process.env.ARDUINO_PATH || '/dev/ttyACM0';
const BAUD = 9600;

const port = new SerialPort({ path: ARDUINO_PATH, baudRate: BAUD });

port.on('open', () => console.log('Serial port open'));
port.on('data', data => console.log('←', data.toString()));

app.post('/cmd', async (req, res) => {
  const { cmd } = req.body;
  if (!cmd) return res.status(400).json({ error: 'cmd required' });
  port.write(cmd + '\n', err => {
    if (err) return res.status(500).json({ error: err.message });
    res.json({ status: 'sent', cmd });
  });
});

app.listen(3000, () => console.log('Server listening 3000'));
```

Client-side (Svelte) then calls your backend `/cmd` endpoint with `fetch('/cmd', { method: 'POST', body: JSON.stringify({ cmd: 'LED_ON' }), headers: {'Content-Type':'application/json'} })`.

---

## Security & deployment notes

- Web Serial requires user interaction for `requestPort()` — you cannot auto-connect silently for security.
- Node backend approach requires access to the server's serial device; if you expose the server publicly, be sure to authenticate and secure the endpoint.
- Always match baud rate and newline conventions between app and Arduino.

---

## Next steps / Customizations

- Add UI to discover and select specific serial devices (Web Serial `getPorts()` / Node `SerialPort.list()`).
- Add structured commands (JSON framed) for more complex interactions.
- Add reconnection logic and more robust parsing of incoming data.


---

If you want, I can:
- produce a fully runnable Vite project ZIP structure,
- convert this to a SvelteKit route,
- or make the UI prettier with Tailwind.

Tell me which and I'll update the canvas.
