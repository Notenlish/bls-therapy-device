<script lang="ts">
  let {
    curMode,
    setMode,
  }: { curMode: "setup" | "pair" | "therapy"; setMode: (m: string) => void } =
    $props();

  const btnData: { text: string; mode: string }[] = [
    { text: "Setup", mode: "setup" },
    { text: "Pair", mode: "pair" },
    { text: "Therapy", mode: "therapy" },
  ];
</script>

<div class="grid grid-cols-3 gap-1 w-full my-2">
  {#each btnData as btn}
    <button
      class={`py-2 rounded ${curMode == btn.mode ? "bg-primary-500" : ""}`}
      onclick={() => {
        setMode(btn.mode);
      }}
      type="button"
      ><span class={`${curMode == btn.mode ? "text-white" : ""}`}
        >{btn.text}</span
      ></button
    >
  {/each}
</div>
