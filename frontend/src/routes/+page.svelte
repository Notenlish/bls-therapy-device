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

<h1>Web Arayüzü</h1>
<p>esp32c3'ü kontrol etmek için olan bir test arayüzü.</p>
<p>TCP sunucusunun başlaması için mesaj göndermelisin.</p>

<p>
  Connected to request handler: {connected}
  <br />
  Latest received: {latestReceived}
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

<style>
  form {
    padding: 0.5rem;
    margin-block: 0.5rem;
    background-color: rgb(225, 225, 225);
  }
</style>
