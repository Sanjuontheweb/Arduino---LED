<script>
    import _ from 'lodash';

    let leds = _.range(0, 8).map(() => _.range(0, 8).map(() => false))

    function onToggleLed(row, column) {
        leds[row][column] = !leds[row][column]
        leds = [...leds]
    }
    $: arduinoMsg = [...leds].reverse().reduce((acc, value) => {
        return acc + value.map(l => l?'1':'0').join(':') + ':';
    }, '').slice(0, -1);
</script>

<style>
    main {
        width: 500px;
        height: 500px;
        margin: 10px auto;
        padding: 6px 0 0 6px;
        border: solid black 1px;
        background: linear-gradient(to right, rgb(173, 88, 88), rgb(87, 23, 23));
    }
    section.row {
        display: flex;
    }
    div.circles {
        width: 50px;
        height: 50px;
        border-radius: 50%;
        margin: 5px;
        border: solid black 1px;
        background-color: #c0bfbf;
        cursor: pointer;
    }
    div.on {
        background-color: #AA0000;
    }
    h1,h2 {
        text-align: center;
        font-weight: 700;
    }
    h2 {
        font-size: 30px;
    }
    section#message {
        width: 500px;
        word-wrap: break-word;
        border: solid black 1px;
        padding: 5px;
        margin: 10px auto;
        font-weight: 700;
    }

</style>

<h1>LED MATRIX [ARDUINO]</h1>

<main>
    <!-- svelte-ignore a11y_click_events_have_key_events -->
    <!-- svelte-ignore a11y_no_static_element_interactions -->
    {#each leds as row, rowIndex}
        <section class="row">
            {#each row as led, columnIndex}
                <div class:on={led} on:click={() => onToggleLed(rowIndex, columnIndex)} class="circles"></div>
            {/each}
        </section>
    {/each}
</main>

<h2>Adruino message</h2>

<section id="message">
    {arduinoMsg}
</section>

<!-- Basically, u create an array 0-8 which each contain another array of 0-8
 loop through each array and  -->