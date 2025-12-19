<script lang="ts">
  let connected = $state(false);
  let latestReceived = $state("");
  let powerLevelToSet = $state(0);

  import { onMount } from "svelte";

  const sendReq = async (cmd_name:string, payload: string = "") => {
    const message = `${cmd_name} ${payload}`;
    const res = await fetch("/api/send-message", {"method":"POST", "body": JSON.stringify({"message":message})});
    if (res.ok) {
      latestReceived = await res.json();
    }
  }


  // only runs in client side
  onMount(() => {
    const intervalId = setInterval(async () => {
      if (!connected) {
        const res = await fetch("/api/connect", { "method": "GET" });
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

<p>
  Connected: {connected}
  Latest received: {latestReceived}
</p>

<form action="/api/" method="post"></form>
<input type="number" bind:value={powerLevelToSet}>
<button onclick={() => {sendReq("SETP", powerLevelToSet.toString())}}> Set battery level</button>

