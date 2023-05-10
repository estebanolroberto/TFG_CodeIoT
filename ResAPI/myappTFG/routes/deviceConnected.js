var express = require("express");
var mongoose = require('mongoose');
var router = express.Router();
var devicesConnected_model = require('../models/DevicesConnected.js');
var Device = require('../models/Devices.js');

var db = mongoose.connection;

/* GET home page. */
router.get("/", function (req, res, next) {
  devicesConnected_model.find().exec(function (err, sensor) {
  if (err) res.status(500).send(err);
  else res.status(200).json(sensor);
});
});



/* GET single data by ID */
router.get('/:id', function (req, res, next) {
    devicesConnected_model.findById(req.params.id, function (err, sensor) {
    if (err) res.status(500).send(err);
    else res.status(200).json(sensor);
  });
});


/* POST a new data */
router.post('/', async (req, res) => {
  try {
    const { direction, actual_frequency, deviceId } = req.body;

    // Buscar el dispositivo relacionado en "Devices" por dirección
    const device = await Device.findOne({ direction });

    if (!device) {
      return res.status(404).json({ error: 'No se encontró el dispositivo relacionado' });
    }

    // Crear el nuevo documento en "DevicesConnected" con la referencia al dispositivo
    const newDeviceConnected = new devicesConnected_model({
      direction,
      actual_frequency,
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
    devicesConnected_model.findById(req.params.id, function (err, sensor) {
    if (err) res.status(500).send(err);
    else res.status(200).json(sensor);
  });
});

/* DELETE data by Id */
router.delete("/:id", function (req, res, next) {
    devicesConnected_model.findByIdAndDelete(req.params.id, function (err, sensor) {
    if (err) res.status(500).send(err);
    else res.sendStatus(200);
  });
});


module.exports = router;
