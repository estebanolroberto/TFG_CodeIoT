var mongoose = require("mongoose");
var Schema = mongoose.Schema;

var DevicesConnectedSchema = new Schema({
  direction: String,
  actual_frequency: String,
  addeddate: { type: Date, default: Date.now },
  device: { type: Schema.Types.ObjectId, ref: 'devices' }
},
{ collection: 'devices_connected' });

module.exports = mongoose.model('devices_connected', DevicesConnectedSchema);
