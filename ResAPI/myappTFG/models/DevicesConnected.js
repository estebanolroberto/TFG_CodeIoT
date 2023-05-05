var mongoose = require("mongoose");
var Schema = mongoose.Schema;

var DevicesConnectedchema = new Schema({
    direction: { type: Schema.ObjectId, ref: 'Devices' },
    addeddate: { type: Date, default: Date.now }
},
{ collection: 'devices_connected' });

// el ObjectId está implicito
module.exports = mongoose.model('devices_connected', DevicesConnectedchema);
