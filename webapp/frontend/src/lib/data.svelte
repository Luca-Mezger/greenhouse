<script>
    import { onMount } from "svelte";
    import { format } from "date-fns";

    let sensorType = "temperature";
    let startDate = "";
    let endDate = "";
    let sensorData = [];

    const API_URL = import.meta.env.VITE_API_URL;

    async function fetchSensorData() {
        if (!startDate || !endDate) {
            alert("Please select both start and end date/time.");
            return;
        }

        const formattedStartDate = format(new Date(startDate), "yyyy-MM-dd HH:mm:ss");
        const formattedEndDate = format(new Date(endDate), "yyyy-MM-dd HH:mm:ss");

        const response = await fetch(
            `${API_URL}/api/sensor_data?sensor_type=${sensorType}&start_date=${formattedStartDate}&end_date=${formattedEndDate}`
        );

        if (response.ok) {
            sensorData = await response.json();
        } else {
            alert("Failed to fetch data. Please check your inputs.");
        }
    }

    function handleFetch() {
        fetchSensorData();
    }
</script>

<h1>Smart Greenhouse Sensor Data</h1>

<!-- Sensor Type Selector -->
<div>
    <label for="sensorType">Sensor Type:</label>
    <select id="sensorType" bind:value={sensorType}>
        <option value="temperature">Temperature</option>
        <option value="humidity_air">Humidity (Air)</option>
        <option value="humidity_soil">Humidity (Soil)</option>
        <option value="light">Light</option>
        <option value="co2_concentration">CO2 Concentration</option>
    </select>
</div>

<!-- Start Date/Time Selector -->
<div>
    <label for="startDate">Start Date/Time:</label>
    <input id="startDate" type="datetime-local" bind:value={startDate} />
</div>

<!-- End Date/Time Selector -->
<div>
    <label for="endDate">End Date/Time:</label>
    <input id="endDate" type="datetime-local" bind:value={endDate} />
</div>

<!-- Fetch Data Button -->
<div>
    <button on:click={handleFetch}>Fetch Data</button>
</div>

<!-- Display the Data -->
{#if sensorData.length > 0}
    <h2>Results:</h2>
    <ul>
        {#each sensorData as data}
            <li>{data.timestamp}: {data.value}</li>
        {/each}
    </ul>
{:else}
    <p>No data available for the selected range.</p>
{/if}
