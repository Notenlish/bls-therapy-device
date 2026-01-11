<script lang="ts">
  import QrScanner from "qr-scanner";

  import { VideoIcon } from "@lucide/svelte";

  let curStep = $state(0);
  let videoElm: HTMLVideoElement = $state();
  let qrScanner;

  const steps: {
    text: string;
    imgPath: string;
    imgAlt: string;
    id?: string;
    requireStep?: boolean;
  }[] = [
    {
      text: "Yeni aldığınız cihazı kutusundan çıkarın",
      imgPath: "",
      imgAlt: "",
    },
    {
      text: "Cihazın arkasındaki QR kodu okutun ve verilen ağ adına katılın:",
      requireStep: true,
      imgPath: "",
      imgAlt: "",
      id: "readQR",
    },
  ];

  let curStepData = $derived(steps[curStep]);
  $effect(() => {
    if (curStepData.id == "readQR") {
      qrScanner = new QrScanner(
        videoElm,
        (result) => console.log("decoded qr code:", result),
        {
          /* your options or returnDetailedScanResult: true if you're not specifying any other options */
        }
      );
    }
  });
</script>

<p>Cihazınızın kurulumunu aşağıdaki adımları takip ederek yapabilirsiniz:</p>

<div
  class="card my-4 preset-filled-surface-100-900 p-4 w-full text-center flex flex-col items-center"
>
  <div class="flex flex-col items-center gap-2">
    <div
      class="relative text-2xl rounded-full aspect-square h-8 grid place-content-center"
    >
      <h1 class="-translate-y-0.5 w-max font-semibold">
        <span class="text-surface-950">Adım {curStep}</span>
      </h1>
    </div>
    <p>{curStepData.text}</p>
  </div>
  <img
    class="w-full min-h-24 border"
    src={curStepData.imgPath}
    alt={curStepData.imgAlt}
  />
  {#if curStepData.id == "readQR"}
    <video bind:this={videoElm}></video>
    <!-- If I'm going to be using skeleton ui kit I should actually be using the components it provides lol. -->
    <button
      onclick={() => {
        if (navigator.mediaDevices.getUserMedia) {
          navigator.mediaDevices
            .getUserMedia({ video: true })
            .then((stream) => {
              videoElm.srcObject = stream;
              videoElm.play();
              videoElm.muted = true;
              videoElm.playsInline = true;
            })
            .catch((error) => {
              console.error("Something went wrong!", error);
            });
        } else {
          console.log("getUserMedia not supported on your browser!");
        }
      }}
      class="bg-primary-500 flex items-center gap-2 text-surface-50 py-2 px-4 rounded"
      type="button"><VideoIcon></VideoIcon>Start Camera</button
    >
    <br />
    VEYA
    <br />
    WiFi
  {/if}

  <div class="grid grid-cols-2 gap-2">
    <button
      class="text-md py-2 px-4 rounded bg-primary-500 text-surface-50"
      onclick={() => {
        curStep -= 1;
      }}
      type="button">Geri</button
    >
    <button
      class="text-md py-2 px-4 rounded bg-primary-500 text-surface-50"
      onclick={() => {
        curStep += 1;
      }}
      type="button">İleri</button
    >
  </div>
</div>
