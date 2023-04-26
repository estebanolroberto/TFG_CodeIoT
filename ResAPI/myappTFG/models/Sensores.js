var mongoose = require("mongoose");
var Schema = mongoose.Schema;

var Sensorchema = new Schema({
    type_connection:String,
    direction:String,
    description:String,
    data_measure:String,
    frequency_data:String,
    min_valueData:String,
    max_valueData:String
},
{ collection: 'sensores' });

// el ObjectId está implicito
module.exports = mongoose.model('sensores', Sensorchema);
