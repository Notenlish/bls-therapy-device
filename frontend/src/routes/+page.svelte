<script lang="ts">
  let connected = $state(false);
  let latestReceived = $state("");
  let powerLevelToSet = $state(0.5);

  import { onMount } from "svelte";

  const sendRequest = async (cmd_name: string, payload: string = "") => {
    const res = await fetch("/api/send-request", {
      method: "POST",
      body: JSON.stringify({ command_name: cmd_name, payload: payload }),
    });
    if (res.ok) {
      latestReceived = await res.json();
    }
  };

  // only runs in client side
  onMount(() => {
    // literally only using this to start the socket server automatically.
    const res = fetch("/api/send-request", {
      method: "POST",
      body: JSON.stringify({ command_name: "NONE", payload: "" }),
    }).then((e) => {
      if (e.ok) {
        e.json().then((e) => (latestReceived = e));
      }
    });

    const intervalId = setInterval(async () => {
      if (!connected) {
        const res = await fetch("/api/connect", { method: "GET" });
        if (res.ok) {
          connected = (await res.json()).connected || false;
        }
      }
    }, 2000);
    return () => clearInterval(intervalId);
  });
</script>

<header><h1>Web Arayüzü</h1></header>
<main>
  <p>esp32c3'ü kontrol etmek için olan bir test arayüzü.</p>

  <p>
    Connected to request handler: <span
      style:color={connected ? "green" : "red"}>{connected}</span
    >
    <br />
    Latest received: {JSON.stringify(latestReceived)}
  </p>

  <div class="control-grid">
    <div class="controller">
      <h2>Motor Güç Seviyesi: {powerLevelToSet}</h2>
      <form
        onsubmit={async (e) => {
          await sendRequest("SETP", powerLevelToSet.toString());
        }}
      >
        <input
          type="range"
          min="0"
          max="1"
          step="0.1"
          bind:value={powerLevelToSet}
        />
        <button type="submit">Motor Gücünü Ayarla</button>
      </form>
    </div>

    <div class="controller">
      <h2>Motoru Başlat</h2>
      <form
        onsubmit={async (e) => {
          await sendRequest("STRT", "");
        }}
      >
        <button type="submit">Başlat</button>
      </form>
    </div>

    <div class="controller">
      <h2>Motoru Durdur</h2>
      <form
        onsubmit={async (e) => {
          await sendRequest("STOP", "");
        }}
      >
        <button type="submit">Durdur</button>
      </form>
    </div>
  </div>
</main>

<style>
  :global(.roboto-font) {
    font-family: "Roboto", sans-serif;
    font-optical-sizing: auto;
    font-style: normal;
    font-variation-settings: "wdth" 100;
  }
  :global(body) {
    margin: 0;
    padding: 0;
  }
  :global(main) {
    margin-inline: 1rem;
  }
  header {
    padding: 1rem;
    margin: 0;
    background-color: rgb(151, 184, 246);
    color: rgba(26, 26, 255);
  }
  p {
    font-size: 18px;
  }
  header h1 {
    padding: 0;
    margin: 0;
  }
  form {
    padding: 0.5rem;
    margin-block: 0.5rem;
    display: flex;
    flex-wrap: wrap;
    gap: 2rem;
  }
  :global(button) {
    border-radius: 3px;
    padding: 0.8rem 1.6rem;
    border: none;
    background-color: rgb(51, 109, 234);
    color: white;
    font-size: 1rem;
    font-weight: 600;
    cursor: pointer;
  }
  :global(button:hover) {
    background-color: rgb(41, 98, 221);
  }

  .control-grid {
    display: grid;
    grid-template-columns: 1fr 1fr;
  }

  .controller {
    margin: 1rem;
    padding: 1rem;
    border: solid 2px rgb(31, 81, 209);
    border-radius: 4px;
  }
  .controller h2 {
    margin: 0;
    font-weight: 500;
    font-size: 1.4rem;
  }
</style>
