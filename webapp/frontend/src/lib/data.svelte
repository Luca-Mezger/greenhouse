<script>
    import { onMount } from "svelte";
    import { format, subDays } from "date-fns";
    import { Chart, registerables } from 'chart.js';
    import 'chartjs-adapter-date-fns';  // Import the date adapter for time scales

    Chart.register(...registerables);

    let sensorType = "soil_humidity";  // Default to soil humidity
    let startDate = "";
    let endDate = "";
    let sensorData = [];
    let chartData = { labels: [], values: [] };
    let chart;  // Reference to the chart instance

    let API_URL = "http://192.168.90.62:5000";

    // Automatically set startDate and endDate to the last 14 days on mount
    onMount(() => {
        const now = new Date();
        endDate = format(now, "yyyy-MM-dd'T'HH:mm");
        startDate = format(subDays(now, 14), "yyyy-MM-dd'T'HH:mm");
        fetchSensorData();  // Fetch data on mount
    });

    async function fetchSensorData() {
        const formattedStartDate = format(new Date(startDate), "yyyy-MM-dd HH:mm:ss");
        const formattedEndDate = format(new Date(endDate), "yyyy-MM-dd HH:mm:ss");

        const response = await fetch(
            `${API_URL}/api_data?sensor_type=${sensorType}&start_date=${formattedStartDate}&end_date=${formattedEndDate}`
        );

        if (response.ok) {
            sensorData = await response.json();
            updateChart(sensorData);
        } else {
            alert("Failed to fetch data. Please check your inputs.");
        }
    }

    // Function to update the chart with new data
    function updateChart(data) {
        // Update chartData
        chartData.labels = data.map(d => d.timestamp);
        chartData.values = data.map(d => d.value);

        // Destroy the old chart if it exists to avoid the "Canvas is already in use" error
        if (chart) {
            chart.destroy();
        }

        // Create a new chart
        const ctx = document.getElementById('myChart').getContext('2d');
        chart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: chartData.labels,
                datasets: [{
                    label: 'Sensor Data',
                    data: chartData.values,
                    borderColor: 'rgba(75, 192, 192, 1)',
                    borderWidth: 2,
                    fill: false,
                    tension: 0.1
                }]
            },
            options: {
                scales: {
                    x: {
                        type: 'time',
                        time: {
                            unit: 'day',  // Adjust the unit as necessary (e.g., 'minute', 'hour', etc.)
                        },
                    },
                    y: {
                        beginAtZero: true
                    }
                }
            }
        });
    }
</script>

<style lang="postcss">
body {
    @apply font-sans;
}

    h1 {
        @apply text-3xl font-bold mb-4;
    }

    .container {
        @apply max-w-4xl mx-auto px-4 py-8;
    }

    label {
        @apply text-gray-700 font-medium;
    }

    select,
    input {
        @apply block w-full px-3 py-2 mb-4 border border-gray-300 rounded-md shadow-sm; focus:outline-none; focus:ring-blue-500; focus:border-blue-500; sm:text-sm;
    }

    .button {
        @apply bg-blue-500 text-white px-4 py-2 rounded; hover:bg-blue-600;
    }

</style>



<div class="container">
    <h1>Smart Greenhouse Sensor Data</h1>

    <!-- Sensor Type Selector -->
    <div>
        <label for="sensorType">Sensor Type:</label>
        <select id="sensorType" bind:value={sensorType}>
            <option value="light_status">Brightness</option>
            <option value="air_humidity">Humidity (Air)</option>
            <option value="soil_humidity">Humidity (Soil)</option>
            <option value="hours_of_light">Hours of Light</option>
            <option value="temperature">Temperature</option>
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
        <button on:click={fetchSensorData}>Fetch Data</button>
    </div>

    <!-- Display the Data in a Graph -->
    <canvas id="myChart" width="400" height="200"></canvas>

    <!-- Display the Data as a List -->
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
</div>
