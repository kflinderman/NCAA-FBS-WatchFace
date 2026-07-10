// Import the Clay package
var Clay = require('@rebble/clay');
// Load our Clay configuration file
var clayConfig = require('./config');
var customClay = require('./customClay');

// Initialize Clay
var clay = new Clay(clayConfig, customClay);