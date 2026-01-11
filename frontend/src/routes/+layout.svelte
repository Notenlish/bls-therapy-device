<script lang="ts">
  import "./layout.css";
  import favicon from "$lib/assets/favicon.svg";

  import Appheader from "../component/appheader.svelte";

  let { children } = $props();
</script>

<svelte:head><link rel="icon" href={favicon} /></svelte:head>
<Appheader title="Web Arayüzü" />

<main class="bg-surface-50 p-4 min-h-screen mb-12">{@render children()}</main>
