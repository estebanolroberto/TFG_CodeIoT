var express = require("express");
var mongoose = require('mongoose');
var router = express.Router();
var Sensores = require('../models/SensorHTU.js');
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


/* POST a new data */
router.post('/', async (req, res) => {
  try {
    const { sensor_type, direction, temperature, humidity, deviceId } = req.body;
    // Buscar el dispositivo relacionado en "Devices" por dirección
    const device = await Device.findOne({ direction });

    if (!device) {
      return res.status(404).json({ error: 'No se encontró el dispositivo relacionado' });
    }

    // Crear el nuevo documento en "DevicesConnected" con la referencia al dispositivo
    const newDeviceConnected = new Sensores({
      sensor_type,
      direction,
      temperature,
      humidity,
      device: device._id  // Establecer la referencia al dispositivo
    });

    const savedDeviceConnected = await newDeviceConnected.save();
    res.json(savedDeviceConnected);
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
