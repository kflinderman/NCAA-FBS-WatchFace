// Import the Clay package
var Clay = require('@rebble/clay');
// Load our Clay configuration file
var clayConfig = require('./config');
var customClay = require('./customClay');

// Initialize Clay
var clay = new Clay(clayConfig, customClay);

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function weatherCodeToCondition(code) {
  if (code === 0) return 0; //'Clear';
  if (code <= 3) return 1; //'Cloudy';
  if (code <= 48) return 2; //'Fog';
  if (code <= 55) return 3; //'Drizzle';
  if (code <= 57) return 4; //'Fz. Drizzle';
  if (code <= 65) return 5; //'Rain';
  if (code <= 67) return 6; //'Fz. Rain';
  if (code <= 75) return 7; //'Snow';
  if (code <= 77) return 8; //'Snow Grains';
  if (code <= 82) return 9; //'Showers';
  if (code <= 86) return 10; //'Snow Shwrs';
  if (code === 95) return 11; //'T-Storm';
  if (code <= 99) return 12; //'T-Storm';
  return 13; //'Unknown';
}

function locationSuccess(pos) {
  var url = 'https://api.open-meteo.com/v1/forecast?' +
      'latitude=' + pos.coords.latitude +
      '&longitude=' + pos.coords.longitude +
      '&current=temperature_2m,weather_code';

  xhrRequest(url, 'GET',
    function(responseText) {
      var json = JSON.parse(responseText);

      var temperature = Math.round(json.current.temperature_2m);
      var conditions = weatherCodeToCondition(json.current.weather_code);

      var dictionary = {
        'TEMPERATURE': temperature,
        'CONDITIONS': conditions
      };

      Pebble.sendAppMessage(dictionary,
        function(e) { console.log('Weather info sent!'); },
        function(e) { console.log('Error sending weather info!'); }
      );
    }
  );
}

function locationError(err) {
  console.log('Error requesting location!');
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    { timeout: 15000, maximumAge: 60000 }
  );
}

Pebble.addEventListener('ready',
  function(e) {
    console.log('PebbleKit JS ready!');
    getWeather();
  }
);

Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
    if (e.payload['REQUEST_WEATHER']) {
      getWeather();
    }
  }
);