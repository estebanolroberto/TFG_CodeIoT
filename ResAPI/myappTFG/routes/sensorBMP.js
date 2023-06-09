var express = require("express");
var mongoose = require('mongoose');
var router = express.Router();
var Sensores = require('../models/SensorBMP.js');
var Device = require('../models/Devices.js');
var db = mongoose.connection;

/* GET home page. */
router.get("/", function (req, res, next) {
    Sensores.find().exec(function (err, sensores) {
    if (err) res.status(500).send(err);
    else res.status(200).json(sensores);
  });
});

/* GET single data by ID */
router.get('/:id', function (req, res, next) {
    Sensores.findById(req.params.id, function (err, sensores) {
    if (err) res.status(500).send(err);
    else res.status(200).json(sensores);
  });
});


/* POST a new data for sensorBMP */
router.post('/', async (req, res) => {
  try {
    const { temperature, pressure, direction, altitude } = req.body;
    // Buscar el dispositivo relacionado en "Devices" por dirección
    const device = await Device.findOne({ direction });

    if (!device) {
      return res.status(404).json({ error: 'No se encontró el dispositivo relacionado' });
    }

    // Preparar el objeto JSON con los datos del sensor BMP
    const data = {
      sensor_type: "BMP",
      direction,
      temperature,
      pressure,
      altitude,
      device: device._id  // Establecer la referencia al dispositivo
    };

    // Guardar los datos en la base de datos (asumiendo que tienes un modelo llamado SensorBMP)
    const newSensorData = new Sensores(data);
    const savedSensorData = await newSensorData.save();

    res.json(savedSensorData);
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

/* PUT data by Id */
router.put("/:id", function (req, res, next) {
    Sensores.findById(req.params.id, function (err, sensores) {
    if (err) res.status(500).send(err);
    else res.status(200).json(sensores);
  });
});

/* DELETE data by Id */
router.delete("/:id", function (req, res, next) {
    Sensores.findByIdAndDelete(req.params.id, function (err, sensores) {
    if (err) res.status(500).send(err);
    else res.sendStatus(200);
  });
});
module.exports = router;
