<script lang="ts">
  import Controller from "./controller.svelte";
  import { onMount } from "svelte";

  let period = $state(4000);
  let betweenWalls = $state(500);
  let wallHitDuration = $state(50);
  let ballDuration = $state(1);
  let ballPathTotalDuration = $derived(2 * wallHitDuration + betweenWalls);
  let ballSpeed = $derived((2.0 * ballPathTotalDuration) / period);
  // svelte-ignore state_referenced_locally
  let ballPosition = $state(ballPathTotalDuration / 2); // initial post is the center of the path
  let pathPercentage = $derived(ballPosition / ballPathTotalDuration); // shouldnt this just be containerDomRect.width * 0.5 or just 0.5, idk.
  let containerDomRect = $state({ height: 0 });
  let containerEl: HTMLElement;
  let yTransform = $derived(containerDomRect.height * 0.5);

  onMount(() => {
    // domRect =
    containerDomRect = containerEl.getBoundingClientRect();
    console.log(containerDomRect);
  });

  // TODO: get dom rect size and automatically get the newer sizes when viewport sizes change
  // TODO: move the ball based on pathPercentage * domrectsize.width
  // TODO: find better colors, either through the built-in svelte color palette generator or through realtimecolors.com
</script>

<Controller>
  <div bind:this={containerEl} class="relative w-full h-full bg-green-50">
    <div
      class="absolute size-16"
      style={`transform: translate(${pathPercentage * ballPosition}px, ${yTransform}px)`}
    >
      <div
        class={`size-16 bg-surface-500 rounded-full -translate-x-1/2 -translate-y-1/2`}
      ></div>
    </div>
  </div>
</Controller>
