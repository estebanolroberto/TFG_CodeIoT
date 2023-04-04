var mongoose = require("mongoose");
var Schema = mongoose.Schema;

var SensorHTUSchema = new Schema({
    sensor_type:String,
    temperature:String,
    humidity:String
},
{ collection: 'sensorHTU' });

// el ObjectId está implicito
module.exports = mongoose.model('sensorHTU', SensorHTUSchema);
