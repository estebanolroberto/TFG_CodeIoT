var express = require("express");
var mongoose = require('mongoose');
var router = express.Router();
var sensores = require('../models/Sensores.js');
var db = mongoose.connection;

/* GET home page. */
router.get("/", function (req, res, next) {
    sensores.find().exec(function (err, sensor) {
    if (err) res.status(500).send(err);
    else res.status(200).json(sensor);
  });
});

/* GET single data by ID */
router.get('/:id', function (req, res, next) {
    sensores.findById(req.params.id, function (err, sensor) {
    if (err) res.status(500).send(err);
    else res.status(200).json(sensor);
  });
});

/* GET single data by address */
router.get('/direction/:id', function (req, res, next) {
  sensores.find({"direction": req.params.id}, function(err,sensor){
    if(err) res.status(500).send(err);
    else res.status(200).json(sensor)
  });
});


/* POST a new data */
router.post("/", function (req, res, next) {
    sensores.create(req.body, function (err, sensor) {
    if (err) res.status(500).send(err);
    else res.sendStatus(200);
  });
});

/* PUT data by Id */
router.put("/:id", function (req, res, next) {
    sensores.findById(req.params.id, function (err, sensor) {
    if (err) res.status(500).send(err);
    else res.status(200).json(sensor);
  });
});

/* DELETE data by Id */
router.delete("/:id", function (req, res, next) {
    sensores.findByIdAndDelete(req.params.id, function (err, sensor) {
    if (err) res.status(500).send(err);
    else res.sendStatus(200);
  });
});
module.exports = router;
