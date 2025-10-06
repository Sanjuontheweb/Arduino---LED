<script>
	import favicon from '$lib/assets/favicon.svg';

	let { children } = $props();
</script>

<div class="mainContainer">
	{@render children?.()}
</div>

<svelte:head>
	<link rel="icon" href={favicon} />
</svelte:head>

<style>
	.mainContainer {
		background-image: linear-gradient(to bottom right, rgb(255, 123, 0), rgb(119, 0, 255));
		color: rgb(0, 0, 0);
		position: relative;
		display: flex;
		flex-direction: column;
		min-height: 100dvh;
		width: 100%;
	}
</style>
