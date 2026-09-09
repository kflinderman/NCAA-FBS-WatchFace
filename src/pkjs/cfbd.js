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
    ESPN_SCOREBOARD_URL: 'https://site.api.espn.com/apis/site/v2/sports/football/college-football/scoreboard?limit=400',
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

  // Unauthenticated HTTP GET - for public, no-key endpoints (ESPN's public
  // scoreboard). Deliberately separate from xhrAuth: no bearer token, and
  // does NOT count against CFBD's monthly usage quota.
  function xhrPublic(url, callback, errorCallback) {
    var xhr = new XMLHttpRequest();
    xhr.timeout = 8000;
    xhr.onload = function() {
      if (xhr.status === 200) {
        try {
          callback(JSON.parse(xhr.responseText));
        } catch (e) {
          console.log('Public request JSON parse error: ' + e);
          if (errorCallback) errorCallback(-1);
        }
      } else {
        console.log('Public request failed: ' + xhr.status + ' ' + url);
        if (errorCallback) errorCallback(xhr.status);
      }
    };
    xhr.onerror = function() {
      console.log('Public request network error: ' + url);
      if (errorCallback) errorCallback(0);
    };
    xhr.ontimeout = function() {
      console.log('Public request timed out: ' + url);
      if (errorCallback) errorCallback(-2);
    };
    xhr.open('GET', url);
    xhr.setRequestHeader('Accept', 'application/json');
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

  // Fetch today's live/final scores from ESPN's public scoreboard. This is
  // an unofficial, undocumented endpoint (no auth, no key, doesn't touch
  // CFBD's quota) - used only for the dedicated live-score poll, since
  // CFBD's free tier only has live scores behind a Patreon-gated
  // /scoreboard endpoint. If this fails or ESPN's shape ever changes, we
  // just return no live data - the watch keeps whatever CFBD last had.
  function fetchEspnLiveScores(callback) {
    xhrPublic(constants.ESPN_SCOREBOARD_URL, function(data) {
      var events = (data && Array.isArray(data.events)) ? data.events : [];
      console.log('ESPN scoreboard: ' + events.length + ' games today');
      callback(events);
    }, function() {
      callback([]);
    });
  }

  // Loose-match team names between CFBD and ESPN's differing conventions
  function normalizeTeamName(name) {
    return (name || '').toLowerCase().replace(/[^a-z0-9]/g, '');
  }

  // Build a per-team lookup (normalized team name -> live game info) from
  // ESPN's raw events, keeping only games that are actually live or final -
  // nothing to report for games that haven't started yet. Indexed by single
  // team name (not a matchup pair) so a per-team live-score request can
  // find a team's own game without already knowing its opponent.
  function buildEspnLiveByTeam(events) {
    var byTeam = {};

    events.forEach(function(event) {
      var comp = event.competitions && event.competitions[0];
      if (!comp || !Array.isArray(comp.competitors)) return;

      var home = comp.competitors.filter(function(c) { return c.homeAway === 'home'; })[0];
      var away = comp.competitors.filter(function(c) { return c.homeAway === 'away'; })[0];
      if (!home || !away) return;

      var status = comp.status || event.status;
      var state = status && status.type && status.type.state; // 'pre' | 'in' | 'post'
      if (state !== 'in' && state !== 'post') return;

      var homeName = (home.team && (home.team.location || home.team.displayName)) || '';
      var awayName = (away.team && (away.team.location || away.team.displayName)) || '';
      if (!homeName || !awayName) return;

      var homePoints = parseInt(home.score, 10) || 0;
      var awayPoints = parseInt(away.score, 10) || 0;
      var completed = !!(status && status.type && status.type.completed);

      byTeam[normalizeTeamName(homeName)] = {
        teamScore: homePoints,
        oppScore: awayPoints,
        completed: completed
      };
      byTeam[normalizeTeamName(awayName)] = {
        teamScore: awayPoints,
        oppScore: homePoints,
        completed: completed
      };
    });

    return byTeam;
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

  // Resolve active season year and calculate upcoming season timestamp boundary.
  // knownNextSeasonTs is whatever the watch already has persisted (0 if
  // unknown) - when it's still in the future we skip the /games lookup
  // entirely and just reuse it.
  function determineSeasonAndBoundaryImpl(apiKey, knownNextSeasonTs, callback) {
    var now = new Date();
    var currentYear = now.getFullYear();
    var nowTs = Math.floor(now.getTime() / 1000);

    console.log('Phase 1: Determine season (current year: ' + currentYear + ')');

    fetchCalendar(currentYear, apiKey, function(calendarResult) {
      var fetchNextSeasonBoundary = function() {
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

    // Resolve season calendar boundary and correct usage quota only.
    // Records/rankings now ride along with light sync instead (see below) -
    // full sync just tracks the season boundary, so it stays rare.
    // knownNextSeasonTs is whatever the watch already has persisted for next
    // season's kickoff (0 if unknown) - lets us skip the /games boundary
    // lookup entirely once it's been learned.
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

          callback({
            year: year,
            nextSeasonFirstGameTs: nextSeasonTs,
            seasonDates: seasonDates,
            weekDates: weekDates,
            apiCallsUsed: usage.used,
            apiCallsLimit: usage.limit
          });
        });
      });

      console.log('=== CFBD Full Sync End ===');
    },

    // Lightweight sync: game schedules, live scores, and records/rankings.
    // targetYear is the exact season year the watch wants games for (its own
    // math, based on the persisted season boundary) - used directly instead
    // of re-deriving it here. knownNextSeasonTs is passed through to the
    // cache-empty fallback below so it can also skip its /games lookup.
    syncLightCFBD: function(apiKey, targetYear, knownNextSeasonTs, callback) {
      function fetchAndReturn() {
        var target = determineCurrentWeek(cache);
        var year = targetYear || cache.currentYear;

        // regular games, [postseason games], records, rankings
        var expected = target.offseason ? 4 : 3;
        var completed = 0;
        var regularGames = [];
        var postGames = [];
        var records = [];
        var rankings = [];

        function onFetchComplete() {
          completed++;
          if (completed !== expected) return;

          callback({
            regularGames: regularGames,
            postGames: postGames,
            inPostseason: target.offseason,
            records: records,
            rankings: rankings,
            apiCallsUsed: usage.used,
            apiCallsLimit: usage.limit
          });
        }

        fetchSeasonGames(year, 'regular', apiKey, function(games) {
          regularGames = games;
          onFetchComplete();
        });

        if (target.offseason) {
          setTimeout(function() {
            fetchSeasonGames(year, 'postseason', apiKey, function(games) {
              postGames = games;
              onFetchComplete();
            });
          }, constants.BATCH_DELAY);
        }

        setTimeout(function() {
          fetchRecords(year, apiKey, function(data) {
            records = data;
            onFetchComplete();
          });
        }, constants.BATCH_DELAY);

        setTimeout(function() {
          fetchRankings(year, target.week, target.offseason, apiKey, function(data) {
            rankings = data;
            onFetchComplete();
          });
        }, constants.BATCH_DELAY * 2);
      }

      if (cache.currentYear === null) {
        console.log('Light sync: cache empty (no full sync this session yet) - determining season first');
        determineSeasonAndBoundary(apiKey, knownNextSeasonTs, function() {
          fetchAndReturn();
        });
        return;
      }
      fetchAndReturn();
    },

    // Dedicated live-score poll: ESPN's public scoreboard only, completely
    // independent of CFBD (no key, no quota impact). Used only while a
    // cached team's game is known to be underway - see the watch-side
    // gating in api.c for when this actually gets called.
    fetchLiveScores: function(callback) {
      fetchEspnLiveScores(function(events) {
        callback(buildEspnLiveByTeam(events));
      });
    },

    normalizeTeamName: normalizeTeamName
  };
})();

module.exports = cfbd;