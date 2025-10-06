<script>
    import _ from 'lodash';

    let port;
    let writer; 
    let status= "disconnected";

    let usbvid = "0x2431";
    let baudrate = "115200";
    let leds = _.range(0, 8).map(() => _.range(0, 8).map(() => false))

    // a textEncoder which makes it easier for arduino to run
    const enc = new TextEncoder();

    function onToggleLed(row, column) {
        leds[row][column] = !leds[row][column]
        leds = [...leds]
    }
    $: arduinoMsg = [...leds].reverse().reduce((acc, value) => {
        return acc + value.map(l => l?'1':'0').join(':') + ':';
    }, '').slice(0, -1) + "$";

    // for this to work, you first need to turn on a flag in chrome : experimental web platform features
    async function connectUsb() {
        if (!navigator.serial) {
            alert("Turn on 'Experimental Web Features' to connect to the arduino.");
            return;
        }

        if (status === "connected") {
            return;
        }
 
        try {
            port = await navigator.serial.requestPort({
                filters: [{ usbVendorId: {usbvid}}],
            });
            await port.open({ baudrate: {baudrate} });
            writer = port.writable.getWriter();
            status = "connected";
        } catch (err) {
            console.log(err);
            alert("There was an error while connecting! I request an Apology");
        }
    }

    $: if (status == "connected"  &&  arduinoMsg != "") {
        writer.write(enc.encode(arduinoMsg));
    }
</script>

<style>
    main {
        width: 500px;
        height: 500px;
        margin: 10px auto;
        padding: 6px 0 0 6px;
        border: solid black 1px;
        background: linear-gradient(to right, rgb(133, 68, 68), rgb(121, 29, 29));
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
    h1 {
        text-align: center;
        font-weight: 700;
    }
    section#message {
        width: 500px;
        word-wrap: break-word;
        border: solid black 1px;
        padding: 5px;
        margin: 10px auto;
        font-weight: 700;
    }

    #usb-connection-info {
        width: 500px;
        margin: 18px auto;
        display:flex;
        flex-direction: column;
        justify-content: center;
        text-align: center;
        font-weight: 700;
        gap: 10px;
    }
    #usb-connection-info h3 {
        letter-spacing: 2px;
    }
    #usb-connection-info button {
        border: none;
        padding: 10px;
        outline: none;
        font-size: 16px;
        cursor: pointer;
        border-radius: 5px;
    }
    #usb-connection-info button:disabled {
        cursor: not-allowed;
    }
    button:active,
    button:focus {
        outline: none;
    }

</style>

<h1>LED MATRIX [ARDUINO]</h1>

<section id="usb-connection-info">
    <button disabled={status === "connected"} on:click={connectUsb}>Connect</button>
    <h3>Status: {status}</h3>
</section>

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

<section id="message">
    {arduinoMsg}
</section>

<!-- Basically, u create an array 0-8 which each contain another array of 0-8
 loop through each array and  -->