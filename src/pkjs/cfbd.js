var cfbd = (function() {
  /**********************/
  /* Cache & Storage    */
  /**********************/

  // In-memory session cache surviving across sync calls
  var cache = {
    currentYear: null,
    nextSeasonFirstGameTs: null,
    seasonDates: [],  // [ startDate, endDate ]
    weekDates: [],    // [[ week, startDate, endDate], ...]
    games: null,
    records: null,
    rankings: null
  };

  var constants = {
    API_BASE: 'https://api.collegefootballdata.com',
    BATCH_DELAY: 100  // ms delay between batched requests
  };

  var seasonDeterminationCallbacks = null;
  var USAGE_STORAGE_KEY = 'cfbd_api_usage';

  /**********************/
  /* Usage Tracking     */
  /**********************/

  // Load API usage history from localStorage
  function loadUsage() {
    try {
      var raw = localStorage.getItem(USAGE_STORAGE_KEY);
      if (raw) return JSON.parse(raw);
    } catch (e) {
      console.log('CFBD usage: failed to load persisted usage: ' + e);
    }
    return null;
  }

  // Persist current API usage stats
  function saveUsage() {
    try {
      localStorage.setItem(USAGE_STORAGE_KEY, JSON.stringify(usage));
    } catch (e) {
      console.log('CFBD usage: failed to persist usage: ' + e);
    }
  }

  var usage = loadUsage() || { year: null, month: null, used: 0, limit: 1000 };

  // Track API call counts and auto-reset on new calendar month
  function trackApiCall() {
    var now = new Date();
    var year = now.getFullYear();
    var month = now.getMonth();

    if (usage.year !== year || usage.month !== month) {
      usage.year = year;
      usage.month = month;
      usage.used = 0;
    }

    usage.used++;
    saveUsage();
  }

  /**********************/
  /* HTTP Request Core  */
  /**********************/

  // Authenticated HTTP GET request handler with bearer auth and usage tracking
  function xhrAuth(url, apiKey, callback, errorCallback) {
    var xhr = new XMLHttpRequest();
    xhr.onload = function() {
      if (xhr.status === 200) {
        try {
          callback(JSON.parse(xhr.responseText));
        } catch (e) {
          console.log('CFBD JSON parse error: ' + e);
          if (errorCallback) errorCallback(-1);
        }
      } else {
        console.log('CFBD request failed: ' + xhr.status + ' ' + url);
        if (errorCallback) errorCallback(xhr.status);
      }
    };
    xhr.onerror = function() {
      console.log('CFBD network error: ' + url);
      if (errorCallback) errorCallback(0);
    };
    xhr.open('GET', url);
    xhr.setRequestHeader('Authorization', 'Bearer ' + apiKey);
    xhr.setRequestHeader('Accept', 'application/json');
    trackApiCall();
    xhr.send();
  }

  /**********************/
  /* Endpoint Fetchers  */
  /**********************/

  // Sync remaining user API quota from CFBD account endpoint
  function fetchUserInfo(apiKey, callback) {
    var url = constants.API_BASE + '/info';
    xhrAuth(url, apiKey, function(data) {
      callback(data);
    }, function(status) {
      console.log('CFBD fetchUserInfo failed: ' + status);
      callback(null);
    });
  }

  // Fetch season schedule boundaries and week date ranges
  function fetchCalendar(year, apiKey, callback) {
    var url = constants.API_BASE + '/calendar?year=' + year;

    xhrAuth(url, apiKey, function(data) {
      if (!data || !Array.isArray(data) || data.length === 0) {
        console.log('No calendar data for year ' + year);
        callback({ isInSeason: false });
        return;
      }

      var now = new Date();
      var startEntry = data[0];
      var endEntry = data[data.length - 1];
      var startStr = startEntry.startDate;
      var endStr = endEntry.endDate;

      var startDate = new Date(startStr);
      var endDate = new Date(endStr);

      if (data.length > 0) {
        cache.seasonDates = [
          data[0].startDate,               // First day of Week 1
          data[data.length - 1].endDate    // Last day of Postseason/Bowl week
        ];
      }

      var inSeason = now >= startDate && now <= endDate;
      var postSeason = now > endDate;

      var weeks = data.map(function(item) {
        return [
          item.week,
          item.startDate,
          item.endDate
        ];
      });

      console.log('Calendar ' + year + ': ' + startStr + ' - ' + endStr +
                  ', in season: ' + inSeason + ', post season: ' + postSeason);

      callback({
        isInSeason: inSeason,
        postSeason: postSeason,
        startDate: startDate,
        endDate: endDate,
        weekDates: weeks
      });
    }, function(status) {
      console.log('fetchCalendar failed: ' + status);
      callback({ isInSeason: false });
    });
  }

  // Strip unneeded fields and filter placeholder games
  function trimGames(data) {
    return data
      .filter(function(game) {
        if (game.homeTeam === 'NA' || game.awayTeam === 'NA') {
          console.log('Skipping game with NA placeholder team');
          return false;
        }
        return true;
      })
      .map(function(game) {
        return {
          startDate: game.startDate,
          homeTeam: game.homeTeam,
          homePoints: game.homePoints,
          awayTeam: game.awayTeam,
          awayPoints: game.awayPoints,
          completed: game.completed
        };
      });
  }

  // Fetch FBS game schedule for specified year and season type
  function fetchSeasonGames(year, seasonType, apiKey, callback) {
    var url = constants.API_BASE + '/games?year=' + year +
        '&seasonType=' + seasonType + '&classification=fbs';
    console.log('Grabbing full ' + seasonType + ' season for ' + year);
    xhrAuth(url, apiKey, function(data) {
      if (!data || !Array.isArray(data)) {
        console.log('No ' + seasonType + ' games for ' + year);
        callback([]);
        return;
      }
      var trimmed = trimGames(data);
      console.log('Fetched ' + trimmed.length + ' ' + seasonType + ' games for ' + year);
      callback(trimmed);
    }, function(status) {
      console.log('fetchSeasonGames failed (' + seasonType + '): ' + status);
      callback([]);
    });
  }

  // Fetch team records filtered down to FBS teams
  function fetchRecords(year, apiKey, callback) {
    var url = constants.API_BASE + '/records?year=' + year;

    xhrAuth(url, apiKey, function(data) {
      if (!data || !Array.isArray(data)) {
        console.log('No records for year ' + year);
        callback([]);
        return;
      }

      var fbsRecords = data
      .filter(function(record) {
        return record.classification === 'fbs';
      })
      .map(function(record) {
        return {
          team: record.team,
          total: {
            games: record.total.games,
            wins: record.total.wins
          },
          postseason: {
            games: record.postseason.games,
            wins: record.postseason.wins,
            losses: record.postseason.losses
          }
        };
      });

      console.log('Fetched records for ' + data.length + ' teams (' + fbsRecords.length + ' FBS)');
      cache.records = fbsRecords;
      callback(fbsRecords);
    }, function(status) {
      console.log('fetchRecords failed: ' + status);
      callback([]);
    });
  }

  // Fetch team rankings (prefers Playoff Committee Rankings, falls back to AP Top 25)
  function fetchRankings(year, week, postseason, apiKey, callback) {
    var seasonTypeStr = postseason ? 'postseason' : 'regular';
    var url = constants.API_BASE + '/rankings?year=' + year + '&seasonType=' + seasonTypeStr + '&week=' + week;

    xhrAuth(url, apiKey, function(data) {
      if (!data || !Array.isArray(data)) {
        console.log('No rankings for year ' + year);
        callback([]);
        return;
      }

      console.log('Fetched ' + data.length + ' ranking entries');

      var selectedPoll = null;

      for (var i = 0; i < data.length; i++) {
        var entry = data[i];
        if (!entry.polls || !Array.isArray(entry.polls)) continue;

        for (var j = 0; j < entry.polls.length; j++) {
          var poll = entry.polls[j];

          if (poll.poll === 'Playoff Committee Rankings') {
            selectedPoll = poll;
            break;
          } else if (poll.poll === 'AP Top 25' && !selectedPoll) {
            selectedPoll = poll;
          }
        }

        if (selectedPoll && selectedPoll.poll === 'Playoff Committee Rankings') break;
      }

      var trimmedRanks = selectedPoll ? selectedPoll.ranks.map(function(entry) {
        return {
          rank: entry.rank,
          school: entry.school
        };
      }) : [];

      console.log('Selected poll: ' + (selectedPoll ? selectedPoll.poll : 'none found') +
                  ' (' + trimmedRanks.length + ' ranks)');

      cache.rankings = trimmedRanks;
      callback(trimmedRanks);
    }, function(status) {
      console.log('fetchRankings failed: ' + status);
      callback([]);
    });
  }

  /**********************/
  /* Boundary Logic     */
  /**********************/

  // Queue simultaneous season resolution calls to avoid redundant requests
  function determineSeasonAndBoundary(apiKey, knownNextSeasonTs, callback) {
    if (seasonDeterminationCallbacks) {
      console.log('Season/boundary determination already in progress - reusing it');
      seasonDeterminationCallbacks.push(callback);
      return;
    }

    seasonDeterminationCallbacks = [callback];
    determineSeasonAndBoundaryImpl(apiKey, knownNextSeasonTs, function(year, nextSeasonTs, seasonDates, weekDates) {
      var callbacks = seasonDeterminationCallbacks;
      seasonDeterminationCallbacks = null;
      for (var i = 0; i < callbacks.length; i++) {
        callbacks[i](year, nextSeasonTs, seasonDates, weekDates);
      }
    });
  }

  // Resolve active season year and calculate upcoming season timestamp boundary
  function determineSeasonAndBoundaryImpl(apiKey, knownNextSeasonTs, callback) {
    var now = new Date();
    var currentYear = now.getFullYear();
    var nowTs = Math.floor(now.getTime() / 1000);

    console.log('Phase 1: Determine season (current year: ' + currentYear + ')');

    fetchCalendar(currentYear, apiKey, function(calendarResult) {
      var fetchNextSeasonBoundary = function() {
        //knownNextSeasonTs is whatever the watch already has persisted (0 if unknown)
        if (knownNextSeasonTs && knownNextSeasonTs > nowTs) {
          console.log('Next season boundary already known (' + knownNextSeasonTs + ') - skipping /games lookup');
          cache.nextSeasonFirstGameTs = knownNextSeasonTs;
          callback(cache.currentYear, cache.nextSeasonFirstGameTs, cache.seasonDates, cache.weekDates);
          return;
        }

        fetchFirstGameOfYear(cache.currentYear + 1, apiKey, function(firstGame) {
          if (firstGame && firstGame.startDate) {
            cache.nextSeasonFirstGameTs = Math.floor(new Date(firstGame.startDate).getTime() / 1000);
          }
          callback(cache.currentYear, cache.nextSeasonFirstGameTs, cache.seasonDates, cache.weekDates);
        });
      };

      if (calendarResult.isInSeason) {
        console.log('Using year ' + currentYear);
        cache.currentYear = currentYear;
        cache.seasonDates[0] = calendarResult.startDate;
        cache.seasonDates[1] = calendarResult.endDate;
        cache.weekDates = calendarResult.weekDates;

        fetchNextSeasonBoundary();
      } else {
        // Fallback check on prior year for postseason or off-season transition
        fetchCalendar(currentYear - 1, apiKey, function(lastYearResult) {
          if (lastYearResult.isInSeason || lastYearResult.postSeason) {
            console.log('In postseason: using year ' + (currentYear - 1));
            cache.currentYear = currentYear - 1;
            cache.seasonDates[0] = lastYearResult.startDate;
            cache.seasonDates[1] = lastYearResult.endDate;
            cache.weekDates = lastYearResult.weekDates;

            fetchNextSeasonBoundary();
          } else {
            console.error('No Schedules Found');
            callback(null, null, null, null);
          }
        });
      }
    });
  }

  // Fetch kickoff game of target season year
  function fetchFirstGameOfYear(year, apiKey, callback) {
    fetchSeasonGames(year, 'regular', apiKey, function(games) {
      if (games.length === 0) {
        callback(null);
        return;
      }
      var sorted = games.slice().sort(function(a, b) {
        return new Date(a.startDate) - new Date(b.startDate);
      });
      callback(sorted[0]);
    });
  }

  // Determine current active season week or offseason status
  function determineCurrentWeek(cache) {
    var now = new Date();
    var nowTs = Math.floor(now.getTime() / 1000);

    var TWO_WEEKS_SECONDS = 14 * 24 * 60 * 60;

    // Branch 1: Within 2 weeks of upcoming season -> advance to new season week 0
    if (cache.nextSeasonFirstGameTs && nowTs >= (cache.nextSeasonFirstGameTs - TWO_WEEKS_SECONDS)) {
      console.log('Within 2 weeks of next season kickoff - using year ' + (cache.currentYear + 1) + ', week 0');
      return {
        year: cache.currentYear + 1,
        week: 0,
        offseason: false
      };
    }

    // Branch 2: Active in-season window
    var seasonStart = new Date(cache.seasonDates[0]);
    var seasonEnd = new Date(cache.seasonDates[1]);

    if (now >= seasonStart && now <= seasonEnd) {
      for (var i = 0; i < cache.weekDates.length; i++) {
        var weekEntry = cache.weekDates[i];
        var weekNum = weekEntry[0];
        var weekStart = new Date(weekEntry[1]);
        var weekEnd = new Date(weekEntry[2]);

        if (now >= weekStart && now <= weekEnd) {
          console.log('In season - matched week ' + weekNum);
          return {
            year: cache.currentYear,
            week: weekNum,
            offseason: false
          };
        }
      }
      var fallbackEntry = cache.weekDates[cache.weekDates.length - 1];
      console.log('In season but between week boundaries - using last known week ' + fallbackEntry[0]);
      return {
        year: cache.currentYear,
        week: fallbackEntry[0],
        offseason: false
      };
    }

    // Branch 3: Offseason
    var lastEntry = cache.weekDates[cache.weekDates.length - 1];
    console.log('Offseason - using last week of ' + cache.currentYear + ': week ' + lastEntry[0] + " Post Season");
    return {
      year: cache.currentYear,
      week: lastEntry[0],
      offseason: true
    };
  }

  /**********************/
  /* Public Module API  */
  /**********************/

  return {
    cache: cache,

    // Perform complete sync: season calendar, usage quota, team records, and rankings
    syncFullCFBD: function(apiKey, knownNextSeasonTs, callback) {
      console.log('=== CFBD Full Sync Start ===');

      determineSeasonAndBoundary(apiKey, knownNextSeasonTs, function(year, nextSeasonTs, seasonDates, weekDates) {
        console.log('=== CFBD Season Boundary Determined ===');

        if (year === null) {
          console.log('Full sync aborted - no season boundary available');
          callback(null);
          return;
        }

        fetchUserInfo(apiKey, function(info) {
          if (info && typeof info.usedCalls === 'number') {
            usage.used = info.usedCalls;
            if (typeof info.monthlyLimit === 'number') {
              usage.limit = info.monthlyLimit;
            }
            var now = new Date();
            usage.year = now.getFullYear();
            usage.month = now.getMonth();
            saveUsage();
            console.log('CFBD usage corrected: ' + usage.used + '/' + usage.limit);
          } else {
            console.log('CFBD usage correction skipped - GET /info unavailable or unlimited plan');
          }

          var target = determineCurrentWeek(cache);

          var expected = 2;
          var completed = 0;
          var records = [];
          var rankings = [];

          function onFetchComplete() {
            completed++;
            if (completed !== expected) return;

            callback({
              year: year,
              nextSeasonFirstGameTs: nextSeasonTs,
              seasonDates: seasonDates,
              weekDates: weekDates,
              records: records,
              rankings: rankings,
              apiCallsUsed: usage.used,
              apiCallsLimit: usage.limit
            });
          }

          fetchRecords(target.year, apiKey, function(data) {
            records = data;
            onFetchComplete();
          });

          setTimeout(function() {
            fetchRankings(target.year, target.week, target.offseason, apiKey, function(data) {
              rankings = data;
              onFetchComplete();
            });
          }, constants.BATCH_DELAY);
        });
      });

      console.log('=== CFBD Full Sync End ===');
    },

    // Lightweight sync: game schedules and live scores
    syncLightCFBD: function(apiKey, targetYear, knownNextSeasonTs, callback) {
      function fetchAndReturn() {
        var target = determineCurrentWeek(cache);
        var year = targetYear || cache.currentYear;

        fetchSeasonGames(year, 'regular', apiKey, function(regularGames) {
          if (!target.offseason) {
            callback({
              regularGames: regularGames,
              postGames: [],
              inPostseason: false,
              apiCallsUsed: usage.used,
              apiCallsLimit: usage.limit
            });
            return;
          }

          fetchSeasonGames(year, 'postseason', apiKey, function(postGames) {
            callback({
              regularGames: regularGames,
              postGames: postGames,
              inPostseason: true,
              apiCallsUsed: usage.used,
              apiCallsLimit: usage.limit
            });
          });
        });
      }

      if (cache.currentYear === null) {
        console.log('Light sync: cache empty (no full sync this session yet) - determining season first');
        determineSeasonAndBoundary(apiKey, knownNextSeasonTs, function() {
          fetchAndReturn();
        });
        return;
      }
      fetchAndReturn();
    }
  };
})();

module.exports = cfbd;