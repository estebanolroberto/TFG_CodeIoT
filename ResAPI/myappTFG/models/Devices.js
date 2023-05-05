var mongoose = require("mongoose");
var Schema = mongoose.Schema;

var Deviceschema = new Schema({
    name:String,
    type_device:String,
    type_connection:String,
    direction:String,
    description:String,
    data_measure:String,
    frequency_data:Number,
    min_valueData:String,
    max_valueData:String,
    units: String,
    manufacturer:String,
    deploymentDate:String,
    interrupt_pin:String,
    max_frecuency:Number,
    min_frecuency:Number
},
{ collection: 'devices' });

// el ObjectId está implicito
module.exports = mongoose.model('devices', Deviceschema);
