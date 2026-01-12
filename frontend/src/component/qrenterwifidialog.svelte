<script lang="ts">
  let { data, onSuccessful } = $props();
  import { XIcon } from "@lucide/svelte";
  import { Dialog, Portal } from "@skeletonlabs/skeleton-svelte";

  const animation =
    "transition transition-discrete opacity-0 translate-y-[100px] starting:data-[state=open]:opacity-0 starting:data-[state=open]:translate-y-[100px] data-[state=open]:opacity-100 data-[state=open]:translate-y-0";
</script>

<Dialog defaultOpen={true}>
  <Dialog.Trigger class="invisible btn preset-filled">Open</Dialog.Trigger>
  <Portal>
    <Dialog.Backdrop class="fixed inset-0 z-50 bg-surface-50-950/80" />
    <Dialog.Positioner
      class="fixed inset-0 z-50 flex justify-center items-center p-4"
    >
      <Dialog.Content
        class="card border border-primary-200 bg-surface-100-900 w-full max-w-xl p-4 space-y-4 shadow-xl {animation}"
        ><header class="flex justify-between items-center">
          <Dialog.Title class="text-lg font-bold"
            >WiFi Bağlantısı Kurma</Dialog.Title
          >
          <Dialog.CloseTrigger class="btn-icon hover:preset-tonal"
            ><XIcon class="size-4"></XIcon></Dialog.CloseTrigger
          >
        </header>
        <Dialog.Description
          >Lütfen bu ağa bağlanın: <span class="bg-primary-50 px-2 py-1 rounded"
            >SetupTherapyDevice-{data.id}</span
          >.</Dialog.Description
        >
        <footer class="flex justify-end gap-2">
          <Dialog.CloseTrigger class="btn-preset-tonal"
            >İptal</Dialog.CloseTrigger
          >
          <button onclick={onSuccessful} type="button" class="btn preset-filled"
            >Bağlandım</button
          >
        </footer>
      </Dialog.Content>
    </Dialog.Positioner>
  </Portal>
</Dialog>
