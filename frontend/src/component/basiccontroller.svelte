<script lang="ts">
  import Controller from "./controller.svelte";

  import { Slider } from "@skeletonlabs/skeleton-svelte";

  let props: {
    motorPower: number;
    setMotorPower: (e: number[]) => void;
  } = $props();
</script>

<Controller>
  <div class="p-4 relative w-full h-full">
    <label class="label flex flex-col gap-2">
      <span class="text-base label-text">Motor Gücü: {props.motorPower}%</span>
      <Slider
        class="pb-4"
        onValueChange={(e) => {
          props.setMotorPower(e.value);
        }}
        defaultValue={[50]}
      >
        <Slider.Control class="rounded-full">
          <Slider.Track
            class="bg-primary-100-900 h-6 border border-surface-200 rounded-full"
          >
            <Slider.Range class="bg-primary-500 rounded-full" />
          </Slider.Track>
          <Slider.Thumb index={0} class="ring-primary-500 hidden">
            <Slider.HiddenInput />
          </Slider.Thumb>
        </Slider.Control>
        <Slider.MarkerGroup>
          <Slider.Marker value={0} />
          <Slider.Marker value={25} />
          <Slider.Marker value={50} />
          <Slider.Marker value={75} />
          <Slider.Marker value={100} />
        </Slider.MarkerGroup>
      </Slider>
    </label>
    <!-- bind:value={powerLevelToSet} -->
  </div>
</Controller>
