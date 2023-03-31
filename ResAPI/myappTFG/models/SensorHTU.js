var mongoose = require("mongoose");
var Schema = mongoose.Schema;

var SensorHTUSchema = new Schema({
    id:String,
    humidity:String,
    temperature:String
},
{ collection: 'sensorHTU' });

// el ObjectId está implicito
module.exports = mongoose.model('sensorHTU', SensorHTUSchema);
