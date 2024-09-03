const express = require('express');
const cors = require('cors');
const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');
const mongoose = require('mongoose');
const WebSocket = require('ws');
require('dotenv').config(); // Load environment variables from a .env file

const app = express();
const port = process.env.PORT || 3000;

// Middleware
app.use(cors());
app.use(express.json()); // To parse JSON bodies

// Initialize Serial Port only if NOT in a cloud environment
let serialPort;
if (process.env.NODE_ENV !== 'production') {
  serialPort = new SerialPort({
    path: process.env.SERIAL_PORT_PATH || 'COM8', // Use environment variable
    baudRate: parseInt(process.env.BAUD_RATE) || 9600,
  });
  const parser = serialPort.pipe(new ReadlineParser({ delimiter: '\n' }));

  // Listen for incoming data from the serial port
  parser.on('data', handleIncomingData);
} else {
  console.warn('Serial port initialization skipped in production environment');
}

// Connect to MongoDB Atlas
mongoose.connect(process.env.MONGODB_URI, { useNewUrlParser: true, useUnifiedTopology: true })
  .then(() => console.log('Connected to MongoDB Atlas'))
  .catch(err => console.error('Could not connect to MongoDB Atlas:', err));

// Define the schema
const sensorSchema = new mongoose.Schema({
  temperature: { type: Number, required: true },
  pressure: { type: Number, required: true },
  altitude: { type: Number, required: true },
  humidity: { type: Number, required: true },
  dhtTemp: { type: Number, required: true },
  accelX: { type: Number, required: true },
  accelY: { type: Number, required: true },
  accelZ: { type: Number, required: true },
  timestamp: { type: Date, default: Date.now }
});

const SensorData = mongoose.model('SensorData', sensorSchema);

// Create HTTP server and WebSocket server
const server = require('http').createServer(app);
const wss = new WebSocket.Server({ server });

// WebSocket server connection
wss.on('connection', (ws) => {
  console.log('New WebSocket connection');

  // Send latest sensor data to new client
  SensorData.find().sort({ timestamp: -1 }).limit(1)
    .then(data => ws.send(JSON.stringify(data[0] || {})))
    .catch(err => ws.send(JSON.stringify({ error: 'Error retrieving data' })));

  // Handle incoming messages from WebSocket clients
  ws.on('message', (message) => {
    console.log('Received message from client:', message);
  });

  // Handle WebSocket errors
  ws.on('error', (error) => {
    console.error('WebSocket error:', error);
  });
});

function handleIncomingData(line) {
  if (line.trim().startsWith('{') && line.trim().endsWith('}')) {
    try {
      const sensorData = JSON.parse(line.trim());
      console.log('Parsed sensor data:', sensorData);

      // Map incoming data to the schema fields
      const mappedData = {
        temperature: sensorData.BMP_Temperature,
        pressure: sensorData.BMP_Pressure,
        altitude: sensorData.BMP_Altitude,
        humidity: sensorData.DHT_Humidity,
        dhtTemp: sensorData.DHT_Temperature,
        accelX: sensorData.Accel_X,
        accelY: sensorData.Accel_Y,
        accelZ: sensorData.Accel_Z
      };

      // Validate the mapped data
      if (Object.values(mappedData).every(value => typeof value === 'number')) {
        const newSensorData = new SensorData(mappedData);
        newSensorData.save()
          .then(() => {
            console.log('Sensor data saved to MongoDB');

            // Broadcast to all WebSocket clients
            wss.clients.forEach((client) => {
              if (client.readyState === WebSocket.OPEN) {
                client.send(JSON.stringify(mappedData));
              }
            });
          })
          .catch(err => console.error('Error saving sensor data:', err));
      } else {
        console.error('Invalid sensor data:', mappedData);
      }
    } catch (err) {
      console.error('Error parsing JSON:', err.message);
    }
  } else {
    console.log('Ignored non-JSON message:', line.trim());
  }
}

// Endpoint to get historical data (e.g., last 24 hours)
app.get('/data/history', (req, res) => {
  const hours = parseInt(req.query.hours) || 24;
  const startTime = new Date();
  startTime.setHours(startTime.getHours() - hours);

  SensorData.find({ timestamp: { $gte: startTime } }).sort({ timestamp: -1 })
    .then(data => res.json(data))
    .catch(err => res.status(500).send('Error retrieving historical data'));
});

// Start the server
server.listen(port, () => {
  console.log(`Server running on http://localhost:${port}`);
});
