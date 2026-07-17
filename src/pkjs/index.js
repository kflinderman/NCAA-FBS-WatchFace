// src/pkjs/index.js
// Import the Clay package
var Clay = require('@rebble/clay');
// Load our Clay configuration file
var clayConfig = require('./config');
var customClay = require('./customClay');
// Load CFBD module
var cfbdModule = require('./cfbd');

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

// Like xhrRequest, but adds a Bearer auth header. Kept separate from the
// existing xhrRequest helper (used for the unauthenticated weather API)
// rather than modifying its shared signature.
var xhrRequestWithAuth = function (url, type, apiKey, callback, errorCallback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    if (xhr.status === 200) {
      callback(xhr.responseText);
    } else {
      console.log('CFBD request failed, status ' + xhr.status);
      if (errorCallback) errorCallback(xhr.status);
    }
  };
  xhr.onerror = function () {
    console.log('CFBD network error');
    if (errorCallback) errorCallback(0);
  };
  xhr.open(type, url);
  xhr.setRequestHeader('Authorization', 'Bearer ' + apiKey);
  xhr.setRequestHeader('Accept', 'application/json');
  xhr.send();
};

// apiKey and teamIndex arrive fresh from the watch on every request (see
// the REQUEST_SCORE branch below) - neither is cached in localStorage or
// held in any module-level variable, so nothing persists key material in
// JS between calls.
function getScore(apiKey, teamIndex) {
  var teamName = CFBD_TEAM_NAMES[teamIndex];
  if (!teamName) {
    console.log('No CFBD team name mapped for index ' + teamIndex);
    return;
  }

  var year = new Date().getFullYear();
  var url = 'https://api.collegefootballdata.com/games?' +
      'year=' + year +
      '&seasonType=regular' +
      '&team=' + encodeURIComponent(teamName);

  xhrRequestWithAuth(url, 'GET', apiKey,
    function (responseText) {
      var games = JSON.parse(responseText);
      if (!games || games.length === 0) {
        console.log('No games returned for ' + teamName);
        return;
      }

      // Prefer the most recently started game (covers "in progress" and
      // "most recently completed" without needing extra date logic here).
      var game = games[games.length - 1];
      sendScoreToWatch(game);
    },
    function (status) {
      console.log('getScore failed for ' + teamName + ' (status ' + status + ')');
    }
  );
}

function sendScoreToWatch(game) {
  var dictionary = {
    'ScoreHomeTeam': (game.homeTeam || '').substring(0, 31),
    'ScoreAwayTeam': (game.awayTeam || '').substring(0, 31),
    'ScoreHomePoints': game.homePoints || 0,
    'ScoreAwayPoints': game.awayPoints || 0,
    'ScoreCompleted': game.completed ? 1 : 0
  };

  Pebble.sendAppMessage(dictionary,
    function(e) { console.log('Score info sent!'); },
    function(e) { console.log('Error sending score info!'); }
  );
}

function sendCFBDDataToWatch(cfbdData) {
  console.log('sendCFBDDataToWatch called with ' + cfbdData.games.length + ' games');

  // Serialize games array (most data-heavy)
  var gamesJson = JSON.stringify(cfbdData.games);
  var recordsJson = JSON.stringify(cfbdData.records);
  var rankingsJson = JSON.stringify(cfbdData.rankings);

  // AppMessage max ~512 bytes usable payload, so chunk aggressively
  var chunkSize = 480;
  var gameChunks = [];
  for (var i = 0; i < gamesJson.length; i += chunkSize) {
    gameChunks.push(gamesJson.substring(i, i + chunkSize));
  }

  console.log('Splitting ' + gamesJson.length + ' chars into ' + gameChunks.length + ' chunks');

  // Send metadata first
  var dictionary = {
    'CFBD_GAMES_TOTAL_CHUNKS': gameChunks.length,
    'CFBD_YEAR': cfbdData.year,
    'CFBD_NEXT_SEASON_TS': cfbdData.nextSeasonFirstGameTs || 0
  };

  Pebble.sendAppMessage(dictionary,
    function(e) {
      console.log('CFBD metadata sent');
      // Send game chunks
      sendGameChunks(gameChunks, 0);
    },
    function(e) {
      console.log('Error sending CFBD metadata!');
    }
  );
}

function sendGameChunks(chunks, index) {
  if (index >= chunks.length) {
    console.log('All game chunks sent');
    return;
  }

  var dictionary = {
    'CFBD_GAMES_CHUNK_INDEX': index,
    'CFBD_GAMES_CHUNK_DATA': chunks[index]
  };

  Pebble.sendAppMessage(dictionary,
    function(e) {
      console.log('Game chunk ' + index + ' sent');
      // Send next chunk with small delay
      setTimeout(function() {
        sendGameChunks(chunks, index + 1);
      }, 50);
    },
    function(e) {
      console.log('Error sending game chunk ' + index + '!');
    }
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
    /*
    if (e.payload['REQUEST_SCORE']) {
      var apiKey = e.payload['api_key'];
      var teamIndex = e.payload['ScoreTeamIndex'];
      if (!apiKey) {
        console.log('REQUEST_SCORE received with no api_key - skipping');
      } else {
        getScore(apiKey, teamIndex);
      }
    }
    */
    // ===== NEW: CFBD Full Sync (typically on app startup or manual refresh) =====
    if (e.payload['REQUEST_CFBD_FULL_SYNC']) {
      var apiKey = e.payload['api_key'];
      if (!apiKey) {
        console.log('REQUEST_CFBD_FULL_SYNC with no api_key - skipping');
        return;
      }

      cfbdModule.syncFullCFBD(apiKey, function(fullData) {
        sendCFBDDataToWatch(fullData);
      });
    }

    // ===== NEW: CFBD Light Sync (weekly games + rankings refresh) =====
    if (e.payload['REQUEST_CFBD_LIGHT_SYNC']) {
      var apiKey = e.payload['api_key'];
      var year = e.payload['cfbd_year'];
      var week = e.payload['cfbd_week'];
      
      if (!apiKey || !year || !week) {
        console.log('REQUEST_CFBD_LIGHT_SYNC missing parameters');
        return;
      }

      cfbdModule.syncLightCFBD(year, week, apiKey, function(lightData) {
        sendCFBDDataToWatch(lightData);
      });
    }
  }
);