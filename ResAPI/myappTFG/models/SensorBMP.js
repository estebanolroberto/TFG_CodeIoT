var mongoose = require("mongoose");
var Schema = mongoose.Schema;

var SensorBMPSchema = new Schema({
    sensor_type:String,
    direction: String,
    temperature:String,
    pressure:String,
    altitude:String,
    device: { type: Schema.Types.ObjectId, ref: 'devices' }
},
{ collection: 'sensorBMP' });

// el ObjectId está implicito
module.exports = mongoose.model('sensorBMP', SensorBMPSchema);
