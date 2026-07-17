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
  console.log('sendCFBDDataToWatch called with ' + cfbdData.games.length + ' games, ' 
    + cfbdData.records.length + ' records, ' + cfbdData.rankings.length + ' rankings');

  var gamesJson = JSON.stringify(cfbdData.games);
  var recordsJson = JSON.stringify(cfbdData.records);
  var rankingsJson = JSON.stringify(cfbdData.rankings);

  console.log('Games: ' + gamesJson.length + ' bytes, Records: ' + recordsJson.length 
    + ' bytes, Rankings: ' + rankingsJson.length + ' bytes');

  // Chunk size for AppMessage
  var chunkSize = 480;
  
  // Create chunk arrays for each data type
  var gameChunks = chunkString(gamesJson, chunkSize);
  var recordChunks = chunkString(recordsJson, chunkSize);
  var rankingChunks = chunkString(rankingsJson, chunkSize);

  console.log('Game chunks: ' + gameChunks.length + ', Record chunks: ' + recordChunks.length 
    + ', Ranking chunks: ' + rankingChunks.length);

  // Send metadata first (includes all three chunk counts)
  var dictionary = {
    'CFBD_YEAR': cfbdData.year,
    'CFBD_NEXT_SEASON_TS': cfbdData.nextSeasonFirstGameTs || 0,
    'CFBD_GAMES_TOTAL_CHUNKS': gameChunks.length,
    'CFBD_RECORDS_TOTAL_CHUNKS': recordChunks.length,
    'CFBD_RANKINGS_TOTAL_CHUNKS': rankingChunks.length
  };

  Pebble.sendAppMessage(dictionary,
    function(e) {
      console.log('CFBD metadata sent');
      // Send all three data types sequentially
      sendGameChunks(gameChunks, 0, function() {
        sendRecordChunks(recordChunks, 0, function() {
          sendRankingChunks(rankingChunks, 0, function() {
            console.log('All CFBD data sent');
          });
        });
      });
    },
    function(e) {
      console.log('Error sending CFBD metadata!');
    }
  );
}

// Helper to break strings into chunks
function chunkString(str, size) {
  var chunks = [];
  for (var i = 0; i < str.length; i += size) {
    chunks.push(str.substring(i, i + size));
  }
  return chunks;
}

// Send game chunks
function sendGameChunks(chunks, index, callback) {
  if (index >= chunks.length) {
    console.log('All game chunks sent');
    if (callback) callback();
    return;
  }

  var dictionary = {
    'CFBD_GAMES_CHUNK_INDEX': index,
    'CFBD_GAMES_CHUNK_DATA': chunks[index]
  };

  Pebble.sendAppMessage(dictionary,
    function(e) {
      console.log('Game chunk ' + index + '/' + chunks.length + ' sent');
      setTimeout(function() {
        sendGameChunks(chunks, index + 1, callback);
      }, 50);
    },
    function(e) {
      console.log('Error sending game chunk ' + index + '!');
    }
  );
}

// Send record chunks (similar structure)
function sendRecordChunks(chunks, index, callback) {
  if (index >= chunks.length) {
    console.log('All record chunks sent');
    if (callback) callback();
    return;
  }

  var dictionary = {
    'CFBD_RECORDS_CHUNK_INDEX': index,
    'CFBD_RECORDS_CHUNK_DATA': chunks[index]
  };

  Pebble.sendAppMessage(dictionary,
    function(e) {
      console.log('Record chunk ' + index + '/' + chunks.length + ' sent');
      setTimeout(function() {
        sendRecordChunks(chunks, index + 1, callback);
      }, 50);
    },
    function(e) {
      console.log('Error sending record chunk ' + index + '!');
    }
  );
}

// Send ranking chunks (similar structure)
function sendRankingChunks(chunks, index, callback) {
  if (index >= chunks.length) {
    console.log('All ranking chunks sent');
    if (callback) callback();
    return;
  }

  var dictionary = {
    'CFBD_RANKINGS_CHUNK_INDEX': index,
    'CFBD_RANKINGS_CHUNK_DATA': chunks[index]
  };

  Pebble.sendAppMessage(dictionary,
    function(e) {
      console.log('Ranking chunk ' + index + '/' + chunks.length + ' sent');
      setTimeout(function() {
        sendRankingChunks(chunks, index + 1, callback);
      }, 50);
    },
    function(e) {
      console.log('Error sending ranking chunk ' + index + '!');
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