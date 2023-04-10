var mongoose = require("mongoose");
var Schema = mongoose.Schema;

var SensorBMPSchema = new Schema({
    sensor_type:String,
    temperature:String,
    pressure:String,
    altitude:String
},
{ collection: 'sensorBMP' });

// el ObjectId está implicito
module.exports = mongoose.model('sensorBMP', SensorBMPSchema);
