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

$sql = "SELECT temperature, timestamp FROM sensor_data ORDER BY id DESC LIMIT 100";
$result = $conn->query($sql);

$data = [];
if ($result->num_rows > 0) {
    while ($row = $result->fetch_assoc()) {
        $data[] = [
            'timestamp' => $row['timestamp'],
            'temperature' => $row['temperature']
        ];
    }
} else {
    echo "No data available";
}
$conn->close();

header('Content-Type: application/json');
echo json_encode($data);
?>