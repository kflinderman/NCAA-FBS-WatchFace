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

function pickTeamGameFromPool(teamName, pool) {
  var matches = pool.filter(function(g) {
    return g.homeTeam === teamName || g.awayTeam === teamName;
  });
  if (matches.length === 0) return null;

  var now = new Date();
  var started = matches.filter(function(g) { return new Date(g.startDate) <= now; });
  if (started.length > 0) {
    started.sort(function(a, b) { return new Date(b.startDate) - new Date(a.startDate); });
    return started[0];
  }

  var upcoming = matches.slice().sort(function(a, b) {
    return new Date(a.startDate) - new Date(b.startDate);
  });
  return upcoming[0];
}

function findLatestTeamGame(teamName, games) {
  if (games.inPostseason) {
    var postGame = pickTeamGameFromPool(teamName, games.postGames);
    if (postGame) return postGame;
    console.log(teamName + ' has no postseason games - falling back to last regular season game');
  }
  return pickTeamGameFromPool(teamName, games.regularGames);
}

function gameToTeamPerspective(teamName, game) {
  if (!game) {
    return { opponent: '', teamScore: 0, vsScore: 0, gametime: 0, completed: false };
  }
  if (game.homeTeam === teamName) {
    return {
      opponent: game.awayTeam || '',
      teamScore: game.homePoints || 0,
      vsScore: game.awayPoints || 0,
      gametime: game.startDate ? Math.floor(new Date(game.startDate).getTime() / 1000) : 0,
      completed: !!game.completed
    };
  }
  return {
    opponent: game.homeTeam || '',
    teamScore: game.awayPoints || 0,
    vsScore: game.homePoints || 0,
    gametime: game.startDate ? Math.floor(new Date(game.startDate).getTime() / 1000) : 0,
    completed: !!game.completed
  };
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

function sendTeamData(teamIndex, teamName, dataType) {
  if (dataType === CFBD_TEAM_DATA_TYPE_GAMES) {
    if (!gamesData) {
      console.log('REQUEST_CFBD_TEAM_DATA (games) received with no games data cached - skipping');
      return;
    }
    var game = findLatestTeamGame(teamName, gamesData);
    var perspective = gameToTeamPerspective(teamName, game);
    var dictionary = {
      'CFBD_TEAM_INDEX': teamIndex,
      'CFBD_TEAM_DATA_TYPE': dataType,
      'CFBD_TEAM_OPPONENT': perspective.opponent.substring(0, 31),
      'CFBD_TEAM_SCORE': perspective.teamScore,
      'CFBD_TEAM_VS_SCORE': perspective.vsScore,
      'CFBD_TEAM_GAMETIME': perspective.gametime,
      'CFBD_TEAM_COMPLETED': perspective.completed ? 1 : 0
    };
    Pebble.sendAppMessage(dictionary,
      function(e) { console.log('Team data (games) sent for index ' + teamIndex + ' (' + teamName + ')'); },
      function(e) { console.log('Error sending team data for index ' + teamIndex + '!'); }
    );
    return;
  }

  if (!recordsRankingsData) {
    console.log('REQUEST_CFBD_TEAM_DATA (records) received with no records/rankings data cached - skipping');
    return;
  }
  var record = findTeamRecord(teamName, recordsRankingsData.records);
  var rank = findTeamRank(teamName, recordsRankingsData.rankings);
  var dictionary = {
    'CFBD_TEAM_INDEX': teamIndex,
    'CFBD_TEAM_DATA_TYPE': dataType,
    'CFBD_TEAM_RANK': rank,
    'CFBD_TEAM_WINS': record.wins,
    'CFBD_TEAM_PS_GAMES': record.postseasonGames,
    'CFBD_TEAM_PS_WINS': record.postseasonWins,
    'CFBD_TEAM_PS_LOSSES': record.postseasonLosses
  };
  Pebble.sendAppMessage(dictionary,
    function(e) { console.log('Team data (records) sent for index ' + teamIndex + ' (' + teamName + ')'); },
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

    if (e.payload['REQUEST_CFBD_LIGHT_SYNC']) {
      var apiKey = e.payload['api_key'];

      if (!apiKey) {
        console.log('REQUEST_CFBD_LIGHT_SYNC missing api_key');
        return;
      }

      cfbdModule.syncLightCFBD(apiKey, function(result) {
        gamesData = result;
        console.log('Games cached: ' + result.regularGames.length + ' regular' +
          (result.inPostseason ? ', ' + result.postGames.length + ' postseason' : ''));

        Pebble.sendAppMessage({
            'CFBD_LIGHT_SYNC_READY': 1,
            'CFBD_API_CALLS_USED': result.apiCallsUsed || 0,
            'CFBD_API_CALLS_LIMIT': result.apiCallsLimit || 0
          },
          function(e) { console.log('Light sync ready signal sent'); },
          function(e) { console.log('Error sending light sync ready signal!'); }
        );
      });
    }

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