<script lang="ts">
  let curStep = $state(0);

  const steps: {
    text: string;
    imgPath: string;
    imgAlt: string;
    requireStep?: boolean;
  }[] = [
    {
      text: "Yeni aldığınız cihazı kutusundan çıkarın",
      imgPath: "",
      imgAlt: "",
    },
    {
      text: "Cihazın arkasındaki QR kodu okutun:",
      requireStep: true,
      imgPath: "",
      imgAlt: "",
    },
  ];
  let curStepData = $derived(steps[curStep]);
</script>

<p>Cihazınızın kurulumunu aşağıdaki adımları takip ederek yapabilirsiniz:</p>

<div class="card my-4 preset-filled-surface-100-900 p-4 w-full text-center">
  <div class="flex flex-col items-center gap-2">
    <div
      class="relative text-2xl rounded-full aspect-square h-8 grid place-content-center"
    >
      <h1 class="-translate-y-0.5 w-max">
        <span class="text-surface-950">Adım {curStep}</span>
      </h1>
    </div>
    <p>{curStepData.text}</p>
  </div>
  <img class="w-full min-h-24 border" src={curStepData.imgPath} alt={curStepData.imgAlt} />
</div>

<ol class="list-decimal ml-4">
  <li>Yeni aldığınız cihazı kutusundan çıkarın</li>
  <li>Web uygulamasında "cihaz kurulumu" sayfasına gidin</li>
  <li>Cihazınızın arkasındaki QR kodunu okutun</li>
  <li>Web sayfası yeni wifi'ye geçmenizi isteyecek.</li>
  <li>SetupTherapyDevice-[XXXXX] adlı geçici wifi ağına bağlanın.</li>
  <li>
    Geçici Wifi ağına bağlandığınız zaman telefonunuz otomatik olarak sizi bir
    web portalına yönlendirecek. Web portalı içerisinden kullandığınız Wi-Fi
    adını ve şifresini giriniz.
  </li>
  <li>"Wi-Fi Bilgilerini Kaydet" tuşuna basın.</li>
  <li>
    Web uygulaması size terapi cihazınızı yeniden başlatmanızı isteyecek. Terapi
    cihazınızı yeniden başlattıktan sonra "tamam" tuşuna tıklayınız.
  </li>
  <li>
    Terapi cihazınızın ışıkları yeniden yeşil yanmaya başladıysa artık terapi
    cihazınızı web uygulamasından kullanabilirsiniz.
  </li>
</ol>
