var mongoose = require("mongoose");
var Schema = mongoose.Schema;

var SensorHTUSchema = new Schema({
    sensor_type:String,
    direction: String,
    temperature:String,
    humidity:String,
    device: { type: Schema.Types.ObjectId, ref: 'devices' }
},
{ collection: 'sensorHTU' });

// el ObjectId está implicito
module.exports = mongoose.model('sensorHTU', SensorHTUSchema);
