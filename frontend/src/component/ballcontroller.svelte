<script lang="ts">
  import Controller from "./controller.svelte";

  let period = $state(4000);
  let betweenWalls = $state(500);
  let wallHitDuration = $state(50);
  let ballDuration = $state(1);
  let ballPathTotalDuration = $derived(2 * wallHitDuration + betweenWalls);
  let ballSpeed = $derived((2.0 * ballPathTotalDuration) / period);
  // svelte-ignore state_referenced_locally
  let ballPosition = $state(ballPathTotalDuration / 2); // initial post is the center of the path
  let pathPercentage = $derived(ballPosition / ballPathTotalDuration);
  
  // TODO: get dom rect size and automatically get the newer sizes when viewport sizes change
  // TODO: move the ball based on pathPercentage * domrectsize.width
</script>

<Controller>
  <div class="relative w-full h-full bg-green-50">
    <div
      class={`absolute size-16 bg-surface-500 rounded-full -translate-x-1/2 -translate-y-1/2 translate-[${}]`}
    ></div>
  </div>
</Controller>
