var mongoose = require("mongoose");
var Schema = mongoose.Schema;

var DevicesConnectedchema = new Schema({
    direction:String,
    time: String
},
{ collection: 'devices_connected' });

// el ObjectId está implicito
module.exports = mongoose.model('devices_connected', DevicesConnectedchema);
