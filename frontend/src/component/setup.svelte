<script lang="ts">
  import QrScanner from "qr-scanner";

  import { VideoIcon } from "@lucide/svelte";
  import Qrenterwifidialog from "./qrenterwifidialog.svelte";

  let curStep = $state(0);
  let videoElm: HTMLVideoElement = $state();
  let qrScanner: QrScanner | undefined = $state();
  let extractedQrData: string | object = $state("");

  const goToNextStep = () => {
    curStep += 1;
  };
  const goToPreviousStep = () => {
    curStep -= 1;
  };

  const steps: {
    text: string;
    imgPath: string;
    imgAlt: string;
    id?: string;
    requireStep?: boolean;
  }[] = [
    {
      text: "Yeni aldığınız cihazı kutusundan çıkarın",
      imgPath: "/setup-steps/0.png",
      imgAlt: "",
    },
    {
      text: "Cihazın arkasındaki QR kodu okutun ve verilen ağ adına katılın:",
      requireStep: true,
      imgPath: "",
      imgAlt: "",
      id: "readQR",
    },
    {
      text: "WiFi'ye bağlandığınız zaman telefonunuz ağda doğrulanmanız için giriş yapmanızı isteyecek. Açılan sayfada kullandığınız iş veya ev wifi'sinin adını ve şifresini girin. Bu şekilde terapi cihazı sizin ağınıza bağlanacak ve cihazınızı kullanabileceksiniz.",
      requireStep: true,
      imgPath: "",
      imgAlt: "",
      id: "openForm",
    },
  ];

  let curStepData = $derived(steps[curStep]);
  $effect(() => {
    if (curStepData.id == "readQR" && extractedQrData) {
    }
  });
</script>

<p>Cihazınızın kurulumunu aşağıdaki adımları takip ederek yapabilirsiniz:</p>

<div
  class="card my-4 preset-filled-surface-100-900 p-4 w-full text-center flex flex-col items-center"
>
  <div
    class="relative text-2xl rounded-full aspect-square h-8 grid place-content-center"
  >
    <h1 class="-translate-y-0.5 w-max font-semibold">
      <span class="text-surface-950">Adım {curStep + 1}</span>
    </h1>
  </div>
  <img
    class="w-full min-h-36 border"
    src={curStepData.imgPath}
    alt={curStepData.imgAlt}
  />
  <br />
  <p>{curStepData.text}</p>
  {#if curStepData.id == "readQR"}
    <div class="qr-grass-theme">
      <video playsinline muted autoplay bind:this={videoElm}></video>
    </div>
    <!-- If I'm going to be using skeleton ui kit I should actually be using the components it provides lol. -->
    <button
      onclick={() => {
        if (navigator.mediaDevices.getUserMedia) {
          navigator.mediaDevices
            .getUserMedia({ video: { facingMode: { ideal: "environment" } } })
            .then((stream) => {
              videoElm.srcObject = stream;
              videoElm.play();

              qrScanner = new QrScanner(
                videoElm,
                (result) => {
                  console.log(result);
                  extractedQrData = JSON.parse(result.data);
                  qrScanner?.stop();
                },
                {
                  highlightScanRegion: true,
                  /* your options or returnDetailedScanResult: true if you're not specifying any other options */
                }
              );
              qrScanner.start();
              console.log("qrscanner", qrScanner);
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
    <b>VEYA</b>
    <br />
    <span>SetupTherapyDevice-[Cihaz No]</span> adlı WiFi'ye bağlanın.
    <br />
    {#if extractedQrData != ""}
      <Qrenterwifidialog onSuccessful={goToNextStep} data={extractedQrData} />
    {/if}
  {/if}
  {#if curStepData.id == "openForm"}
    <br />
    <p>WiFi adını ve şifresini girdikten sonra sonraki adıma geçebilirsiniz.</p>
  {/if}

  <br />
  <div class="grid grid-cols-2 gap-2">
    <button
      class="text-md py-2 px-4 rounded bg-primary-500 text-surface-50"
      onclick={goToPreviousStep}
      disabled={curStep == 0 ? true : false}
      type="button">Geri</button
    >
    <button
      class="text-md py-2 px-4 rounded bg-primary-500 text-surface-50"
      onclick={goToNextStep}
      disabled={curStep == steps.length - 1 ? true : false}
      type="button">İleri</button
    >
  </div>
</div>
