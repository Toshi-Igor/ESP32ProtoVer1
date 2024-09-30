const express = require('express');
const WebSocket = require('ws');
const path = require('path');
const mysql = require('mysql2');

// Create a MySQL connection
const db = mysql.createConnection({
  host: 'localhost',
  user: 'root',
  password: 'iamgRoot',
  database: 'sensor_db'
});

db.connect((err) => {
  if (err) throw err;
  console.log('Connected to MySQL database');
});

const app = express();
const wss = new WebSocket.Server({ port: 8080 });

// Serve static files from the 'public' directory
app.use(express.static(path.join(__dirname, 'public')));

// WebSocket event handling
wss.on('connection', (ws) => {
  console.log('New WebSocket connection');
  
  ws.on('message', (data) => {
    const jsonData = JSON.parse(data);

    const { deviceId, temperature, humidity, heat_index } = jsonData;

    console.log(`Data from ${deviceId} - Temp: ${temperature}, Humidity: ${humidity}, Heat Index: ${heat_index}`);

    // Insert data into MySQL
    const query = 'INSERT INTO dht_data (temperature, humidity, heat_index, deviceId) VALUES (?, ?, ?, ?)';
    db.query(query, [temperature, humidity, heat_index, deviceId], (err, result) => {
      if (err) throw err;
      console.log('Data inserted into database');
    });

    // Broadcast data to all connected clients
    wss.clients.forEach((client) => {
      if (client.readyState === WebSocket.OPEN) {
        client.send(data);
      }
    });
  });
});

// Serve the frontend at the root URL '/'
app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

// Start the HTTP server on port 3000
app.listen(3000, () => {
  console.log('Server running on http://localhost:3000');
});
