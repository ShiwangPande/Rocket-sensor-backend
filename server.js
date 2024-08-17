const express = require('express');
const cors = require('cors'); // For handling CORS
const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');

const app = express();
const port = 3000;

// Setup CORS
app.use(cors());

// Configure SerialPort
const serialPort = new SerialPort({
  path: 'COM8', // Replace 'COM8' with your Arduino's port
  baudRate: 9600,
});

const parser = serialPort.pipe(new ReadlineParser({ delimiter: '\n' }));

// Sensor data object
let sensorData = {
  temperature: null,
  pressure: null,
  altitude: null,
  humidity: null,
  dhtTemp: null,
};

// Parse incoming data from Arduino
parser.on('data', (line) => {
  console.log(`Received: ${line}`);

  // Parse BMP280 sensor data
  if (line.startsWith('Temperature =')) {
    sensorData.temperature = parseFloat(line.split('=')[1].trim().replace(' *C', ''));
  } else if (line.startsWith('Pressure =')) {
    sensorData.pressure = parseFloat(line.split('=')[1].trim().replace(' Pa', ''));
  } else if (line.startsWith('Approx altitude =')) {
    sensorData.altitude = parseFloat(line.split('=')[1].trim().replace(' m', ''));
  }

  // Parse DHT11 sensor data
  if (line.startsWith('Humidity:')) {
    const parts = line.split(' ');
    sensorData.humidity = parseFloat(parts[1].replace('%', ''));

    // Extract DHT temperature
    const tempIndex = line.indexOf('Temperature:');
    if (tempIndex !== -1) {
      const tempStart = tempIndex + 'Temperature: '.length;
      const tempEnd = line.indexOf('°C', tempStart);
      if (tempEnd !== -1) {
        sensorData.dhtTemp = parseFloat(line.substring(tempStart, tempEnd).trim());
      }
    }
  }
});

// API endpoint to get sensor data
app.get('/data', (req, res) => {
  res.json(sensorData);
});

// Start server
app.listen(port, () => {
  console.log(`Server running on http://localhost:${port}`);
});
