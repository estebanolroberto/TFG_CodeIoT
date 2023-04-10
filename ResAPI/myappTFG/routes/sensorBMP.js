var express = require("express");
var mongoose = require('mongoose');
var router = express.Router();
var Sensores = require('../models/SensorBMP.js');
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
router.post("/", function (req, res, next) {
    Sensores.create(req.body, function (err, sensores) {
    if (err) res.status(500).send(err);
    else res.sendStatus(200);
  });
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
