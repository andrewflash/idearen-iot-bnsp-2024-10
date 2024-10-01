<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>IoT Temperature Monitoring Dashboard</title>
    <link href="css/bootstrap.min.css" rel="stylesheet">
    <style>
        .container {
            margin-top: 20px;
        }

        .card {
            margin-bottom: 20px;
        }
    </style>
</head>

<body>
    <div class="container">
        <h1 class="text-center">IoT Temperature Monitoring Dashboard</h1>
        <div class="row">
            <div class="col-md-6">
                <div class="card">
                    <div class="card-header">
                        <h4>Current Temperature</h4>
                    </div>
                    <div class="card-body">
                        <?php
                        $servername = "localhost";
                        $username = "your_username";
                        $password = "your_password";
                        $dbname = "your_database";

                        // Create connection
                        $conn = new mysqli($servername, $username, $password, $dbname);
                        if ($conn->connect_error) {
                            die("Connection failed: " . $conn->connect_error);
                        }

                        $sql = "SELECT temperature, timestamp FROM sensor_data ORDER BY id DESC LIMIT 1";
                        $result = $conn->query($sql);

                        if ($result->num_rows > 0) {
                            while ($row = $result->fetch_assoc()) {
                                echo "<h3>" . $row["temperature"] . " &#8451;</h3>";
                                echo "<p>Last updated: " . $row["timestamp"] . "</p>";
                            }
                        } else {
                            echo "<p>No data available</p>";
                        }
                        $conn->close();
                        ?>
                    </div>
                </div>
            </div>
            <div class="col-md-6">
                <div class="card">
                    <div class="card-header">
                        <h4>Temperature History</h4>
                    </div>
                    <div class="card-body">
                        <canvas id="temperatureChart" width="400" height="200"></canvas>
                        <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
                        <script>
                            var ctx = document.getElementById('temperatureChart').getContext('2d');
                            var temperatureChart = new Chart(ctx, {
                                type: 'line',
                                data: {
                                    labels: [],
                                    datasets: [{
                                        label: 'Temperature (°C)',
                                        data: [],
                                        backgroundColor: 'rgba(75, 192, 192, 0.2)',
                                        borderColor: 'rgba(75, 192, 192, 1)',
                                        borderWidth: 1
                                    }]
                                },
                                options: {
                                    scales: {
                                        x: {
                                            type: 'time',
                                            time: {
                                                unit: 'minute'
                                            }
                                        },
                                        y: {
                                            beginAtZero: true
                                        }
                                    }
                                }
                            });

                            fetch('data.php')
                                .then(response => response.json())
                                .then(data => {
                                    data.forEach(entry => {
                                        temperatureChart.data.labels.push(entry.timestamp);
                                        temperatureChart.data.datasets[0].data.push(entry.temperature);
                                    });
                                    temperatureChart.update();
                                });
                        </script>
                    </div>
                </div>
            </div>
        </div>
    </div>
    <script src="js/bootstrap.min.js"></script>
</body>

</html>