<script lang="ts">
  let connected = $state(false);
  let latestReceived = $state("");
  let powerLevelToSet = $state(0);
  let messageToSend = $state("");

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
  <p>TCP sunucusunun başlaması için mesaj göndermelisin.</p>

  <p>
    Connected to request handler: {connected}
    <br />
    Latest received: {JSON.stringify(latestReceived)}
  </p>

  START:
  <form
    onsubmit={async (e) => {
      await sendRequest("STRT", messageToSend);
    }}
  >
    <input type="text" bind:value={messageToSend} />
    <button type="submit">Send a start message</button>
  </form>

  STOP:
  <form
    onsubmit={async (e) => {
      await sendRequest("STOP", messageToSend);
    }}
  >
    <input type="text" bind:value={messageToSend} />
    <button type="submit">Send a stop message</button>
  </form>

  <input type="number" bind:value={powerLevelToSet} />
  <button
    onclick={() => {
      sendRequest("SETP", powerLevelToSet.toString());
    }}
  >
    Set battery level</button
  >
</main>

<style>
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
    background-color: rgb(225, 225, 225);
  }
</style>
