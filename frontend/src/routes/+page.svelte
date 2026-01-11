<script lang="ts">
  let connected = $state(false);
  let latestReceived = $state("");
  let powerLevelToSet = $state(0.5);
  let motorPower = $state(50);
  let setMotorPower = (e: number[]) => {
    motorPower = e[0];
  };

  let curmode: string | "setup" | "pair" | "therapy" = $state("setup");

  import BallController from "../component/ballcontroller.svelte";
  import Modeswitcher from "../component/modeswitcher.svelte";
  import Setup from "../component/setup.svelte";

  import { onMount } from "svelte";
  import Controllergrid from "../component/controllergrid.svelte";
  import Basiccontroller from "../component/basiccontroller.svelte";

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

<div>
  <Modeswitcher
    setMode={(s: string) => {
      curmode = s;
    }}
    curMode={curmode}
  />
  {#if curmode == "setup"}
    <Setup />
  {/if}
  <p>esp32c3'ü kontrol etmek için olan bir test arayüzü.</p>

  <p>
    Connected to request handler: <span
      style:color={connected ? "green" : "red"}>{connected}</span
    >
    <br />
    Latest received: {JSON.stringify(latestReceived)}
  </p>

  {#if curmode == "therapy"}
  <Controllergrid>
    <Basiccontroller {motorPower} {setMotorPower}></Basiccontroller>
    <BallController></BallController>
  </Controllergrid>
  {/if}

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
</div>
