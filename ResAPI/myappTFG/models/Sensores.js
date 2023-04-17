var mongoose = require("mongoose");
var Schema = mongoose.Schema;

var Sensorchema = new Schema({
    name:String,
    type_connection:String,
    direction:String,
    description:String,
    frequency_data:String,
    min_valueData:String,
    max_valueData:String
},
{ collection: 'sensores' });

// el ObjectId está implicito
module.exports = mongoose.model('sensores', Sensorchema);
