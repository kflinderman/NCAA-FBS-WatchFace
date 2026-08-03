// src/pkjs/index.js

/**
 * Pebble's AppMessage outbox only allows one message in flight at a time -
 * calling sendAppMessage again before the previous call's success/error
 * callback has fired can silently fail or drop a message. During an ~8s
 * CFBD light sync (many small team-by-team round trips), that collision
 * window is wide open, so anything else that sends a message in that
 * window - a weather push, or Clay pushing new settings after its config
 * webview closes - can interrupt the sync.
 *
 * This wraps Pebble.sendAppMessage with a FIFO queue: every call (ours or
 * Clay's own internal one) goes through the same queue and only the head
 * of the queue is ever actually in flight. Placed here, before Clay is
 * even required below, so Clay's internal sendAppMessage calls go through
 * this wrapped version too - not just this file's own calls.
 */
(function() {
  var originalSendAppMessage = Pebble.sendAppMessage.bind(Pebble);
  var sendQueue = [];
  var sending = false;

  function processQueue() {
    if (sending || sendQueue.length === 0) return;
    sending = true;
    var next = sendQueue.shift();
    originalSendAppMessage(next.dict,
      function(e) {
        sending = false;
        if (next.onSuccess) next.onSuccess(e);
        processQueue();
      },
      function(e) {
        sending = false;
        if (next.onError) next.onError(e);
        processQueue();
      }
    );
  }

  Pebble.sendAppMessage = function(dict, onSuccess, onError) {
    sendQueue.push({ dict: dict, onSuccess: onSuccess, onError: onError });
    processQueue();
  };
})();

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

// Two independent caches now, matching the split sync protocol: games
// come from light sync (its own cadence), records+rankings come from full
// sync (records/rankings don't change fast enough within a week to need
// their own cadence, so they ride along with the daily full sync).
// REQUEST_CFBD_TEAM_DATA is served from whichever of these matches the
// requested type - no new API call per team either way, just filtering
// data already fetched by whichever sync last ran.
var gamesData = null;
var recordsRankingsData = null;

// Must match CFBDTeamDataType in api.c
var CFBD_TEAM_DATA_TYPE_GAMES = 0;
var CFBD_TEAM_DATA_TYPE_RECORDS = 1;

function sendCalendarToWatch(calendarData) {
  var dictionary = {
    'CFBD_YEAR': calendarData.year,
    'CFBD_NEXT_SEASON_TS': calendarData.nextSeasonFirstGameTs || 0,
    'CFBD_API_CALLS_USED': calendarData.apiCallsUsed || 0,
    'CFBD_API_CALLS_LIMIT': calendarData.apiCallsLimit || 0
  };

  Pebble.sendAppMessage(dictionary,
    function(e) { console.log('CFBD calendar sent'); },
    function(e) { console.log('Error sending CFBD calendar!'); }
  );
}

// Finds this team's game (if any) in the cached games array and returns
// { opponent, teamScore, vsScore, gametime } from that team's own point
// of view, regardless of whether it played home or away. Returns
// nulls/zeros/empty string if the team has no game this week (bye week).
function findTeamGame(teamName, games) {
  for (var i = 0; i < games.length; i++) {
    var g = games[i];
    if (g.homeTeam === teamName) {
      return {
        opponent: g.awayTeam || '',
        teamScore: g.homePoints || 0,
        vsScore: g.awayPoints || 0,
        gametime: g.startDate ? Math.floor(new Date(g.startDate).getTime() / 1000) : 0
      };
    }
    if (g.awayTeam === teamName) {
      return {
        opponent: g.homeTeam || '',
        teamScore: g.awayPoints || 0,
        vsScore: g.homePoints || 0,
        gametime: g.startDate ? Math.floor(new Date(g.startDate).getTime() / 1000) : 0
      };
    }
  }
  return { opponent: '', teamScore: 0, vsScore: 0, gametime: 0 };
}

function findTeamRecord(teamName, records) {
  for (var i = 0; i < records.length; i++) {
    if (records[i].team === teamName) {
      return {
        wins: records[i].total.wins || 0,
        postseasonGames: records[i].postseason.games || 0,
        postseasonWins: records[i].postseason.wins || 0,
        postseasonLosses: records[i].postseason.losses || 0
      };
    }
  }
  return { wins: 0, postseasonGames: 0, postseasonWins: 0, postseasonLosses: 0 };
}

function findTeamRank(teamName, rankings) {
  for (var i = 0; i < rankings.length; i++) {
    if (rankings[i].school === teamName) {
      return rankings[i].rank || 0;
    }
  }
  return 0;
}

// Handles one REQUEST_CFBD_TEAM_DATA from the watch: looks up teamName in
// whichever cache matches dataType (no new API call, just filtering data
// already fetched by that sync type) and sends back only the field subset
// relevant to that type - the watch applies whatever fields are present,
// so a games response never touches record/ranking fields and vice versa.
function sendTeamData(teamIndex, teamName, dataType) {
  var dictionary = {
    'CFBD_TEAM_INDEX': teamIndex,
    'CFBD_TEAM_DATA_TYPE': dataType
  };

  if (dataType === CFBD_TEAM_DATA_TYPE_GAMES) {
    if (!gamesData) {
      console.log('REQUEST_CFBD_TEAM_DATA (games) received with no games data cached - skipping');
      return;
    }
    var game = findTeamGame(teamName, gamesData.games);
    dictionary['CFBD_TEAM_OPPONENT'] = game.opponent.substring(0, 31);
    dictionary['CFBD_TEAM_SCORE'] = game.teamScore;
    dictionary['CFBD_TEAM_VS_SCORE'] = game.vsScore;
    dictionary['CFBD_TEAM_GAMETIME'] = game.gametime;
  } else {
    if (!recordsRankingsData) {
      console.log('REQUEST_CFBD_TEAM_DATA (records) received with no records/rankings data cached - skipping');
      return;
    }
    var record = findTeamRecord(teamName, recordsRankingsData.records);
    var rank = findTeamRank(teamName, recordsRankingsData.rankings);
    dictionary['CFBD_TEAM_RANK'] = rank;
    dictionary['CFBD_TEAM_WINS'] = record.wins;
    dictionary['CFBD_TEAM_PS_GAMES'] = record.postseasonGames;
    dictionary['CFBD_TEAM_PS_WINS'] = record.postseasonWins;
    dictionary['CFBD_TEAM_PS_LOSSES'] = record.postseasonLosses;
  }

  Pebble.sendAppMessage(dictionary,
    function(e) { console.log('Team data (type ' + dataType + ') sent for index ' + teamIndex + ' (' + teamName + ')'); },
    function(e) { console.log('Error sending team data for index ' + teamIndex + '!'); }
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
    // ===== CFBD Full Sync: calendar + records + rankings =====
    if (e.payload['REQUEST_CFBD_FULL_SYNC']) {
      var apiKey = e.payload['api_key'];
      if (!apiKey) {
        console.log('REQUEST_CFBD_FULL_SYNC with no api_key - skipping');
        return;
      }

      cfbdModule.syncFullCFBD(apiKey, function(fullData) {
        sendCalendarToWatch(fullData);

        recordsRankingsData = fullData;
        console.log('Records/rankings cached: ' + fullData.records.length + ' records, '
          + fullData.rankings.length + ' rankings');

        Pebble.sendAppMessage({
            'CFBD_RECORDS_SYNC_READY': 1,
            'CFBD_API_CALLS_USED': fullData.apiCallsUsed || 0,
            'CFBD_API_CALLS_LIMIT': fullData.apiCallsLimit || 0
          },
          function(e) { console.log('Records/rankings ready signal sent'); },
          function(e) { console.log('Error sending records/rankings ready signal!'); }
        );
      });
    }

    // ===== CFBD Light Sync: fetch this week's games ONCE, cache in
    // memory, then tell the watch it's ready. The watch then requests one
    // team at a time (REQUEST_CFBD_TEAM_DATA below), each served from
    // this same cached fetch - no repeat API calls. =====
    if (e.payload['REQUEST_CFBD_LIGHT_SYNC']) {
      var apiKey = e.payload['api_key'];

      if (!apiKey) {
        console.log('REQUEST_CFBD_LIGHT_SYNC missing api_key');
        return;
      }

      cfbdModule.syncLightCFBD(apiKey, function(lightData) {
        gamesData = lightData;
        console.log('Games cached: ' + lightData.games.length + ' games');

        Pebble.sendAppMessage({
            'CFBD_LIGHT_SYNC_READY': 1,
            'CFBD_API_CALLS_USED': lightData.apiCallsUsed || 0,
            'CFBD_API_CALLS_LIMIT': lightData.apiCallsLimit || 0
          },
          function(e) { console.log('Light sync ready signal sent'); },
          function(e) { console.log('Error sending light sync ready signal!'); }
        );
      });
    }

    // ===== Watch requesting one team's data at a time, post-sync =====
    if (e.payload['REQUEST_CFBD_TEAM_DATA']) {
      var teamIndex = e.payload['CFBD_TEAM_INDEX'];
      var teamName = e.payload['CFBD_TEAM_NAME'];
      var dataType = e.payload['CFBD_TEAM_DATA_TYPE'];

      if (teamName === undefined || teamName === null || teamName === '') {
        console.log('REQUEST_CFBD_TEAM_DATA missing team name for index ' + teamIndex);
        return;
      }

      sendTeamData(teamIndex, teamName, dataType);
    }
  }
);