/**
 * CFBD API module for batched data fetching
 * Handles season detection, data aggregation, and API call optimization
 */

var cfbd = (function() {
  // In-memory cache for this session (survives across multiple calls)
  var cache = {
    currentYear: null,
    nextSeasonFirstGameTs: null,
    seasonDates: [],  // [ startDate, endDate ]
    weekDates: [], // [[ week, startDate, endDate], [week...]]
    games: null,
    records: null,
    rankings: null
  };

  var constants = {
    API_BASE: 'https://api.collegefootballdata.com',
    BATCH_DELAY: 100  // ms between requests to avoid hammering API
  };

  /**
   * Helper: XHR with Bearer auth (from index.js, duplicated for isolation)
   */
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
    xhr.send();
  }

  /**
   * GET /calendar for a year to determine if we're in season
   * Returns: { isInSeason: bool, startDate: Date, endDate: Date, week: int }
   */
  function fetchCalendar(year, apiKey, callback) {
    var url = constants.API_BASE + '/calendar?year=' + year;
    
    xhrAuth(url, apiKey, function(data) {
      if (!data || !Array.isArray(data) || data.length === 0) {
        console.log('No calendar data for year ' + year);
        callback({ isInSeason: false });
        return;
      }

      //for (let i = 0; i < data.length; i++) {
        //console.log("Calendar output: " + JSON.stringify(data[i], null, 2));
      //}
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
          data[data.length - 1].endDate    // Last day of the Postseason/Bowl week
        ];
      }
      
      var inSeason = now >= startDate && now <= endDate;
      var postSeason = now > endDate;
      //let  weeks = [];
      //for (let i = 0; i < data.length; i++) {
        //weeks.push([
          //data[i].week,
          //data[i].startDate,
          //data[i].endDate
        //]);
      //}
      
      const weeks = data.map(item => [
        item.week, 
        item.startDate, 
        item.endDate
      ]);
      

      console.log('Calendar ' + year + ': ' + startStr + ' - ' + endStr + 
                  ', in season: ' + inSeason + ', post season: ' + postSeason);// + ', week: ' + currentWeek);

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

  /**
   * GET /games for a specific year/week to find first game
   * Returns: [ { id, startDate, homeTeam, awayTeam, ... }, ... ]
   */
  function fetchGames(year, week, postseason, apiKey, callback) {
    if (postseason){
      var url = constants.API_BASE + '/games?year=' + year + '&week=' + week + '&seasonType=postseason&classification=fbs';
    }
    else{
      var url = constants.API_BASE + '/games?year=' + year + '&week=' + week + '&seasonType=regular&classification=fbs';
    }
    console.log('Grabbing game data for: ' + year + ' week ' + week + (postseason ? ' (offseason)' : ''));
    xhrAuth(url, apiKey, function(data) {
      if (!data || !Array.isArray(data)) {
        console.log('No games for ' + year + ' week ' + week);
        callback([]);
        return;
      }

      // Sort by startDate to find earliest game
      data.sort(function(a, b) {
        return new Date(a.startDate) - new Date(b.startDate);
      });

      var trimmedGames = data.map(function(game) {
        return {
          startDate: game.startDate,
          homeTeam: game.homeTeam,
          homePoints: game.homePoints,
          awayTeam: game.awayTeam,
          awayPoints: game.awayPoints
        };
      });

      console.log('Fetched ' + trimmedGames.length + ' games for week ' + week);
      callback(trimmedGames);
    }, function(status) {
      console.log('fetchGames failed for week ' + week + ': ' + status);
      callback([]);
    });
  }

  /**
   * GET /games for all weeks of a year (or filter by team)
   * For batch processing; kept separate so it can be called independently
   */
  function fetchAllGamesForYear(year, apiKey, callback) {
    var url = constants.API_BASE + '/games?year=' + year + '&classification=fbs';
    
    xhrAuth(url, apiKey, function(data) {
      if (!data || !Array.isArray(data)) {
        console.log('No games for year ' + year);
        callback([]);
        return;
      }
      console.log('Fetched ' + data.length + ' total games for ' + year);
      cache.games = data;
      callback(data);
    }, function(status) {
      console.log('fetchAllGamesForYear failed: ' + status);
      callback([]);
    });
  }

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

  /**
   * GET /rankings for a year (current/latest poll)
   * Returns: [ { year, week, poll, ranks: [ { rank, team, ... }, ... ] }, ... ]
   */
  function fetchRankings(year, week, postseason, apiKey, callback) {
    if (postseason){
      var url = constants.API_BASE + '/rankings?year=' + year + '&seasonType=postseason&week=' + week;
    }
    else{
      var url = constants.API_BASE + '/rankings?year=' + year + '&seasonType=regular&week=' + week;
    }

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

  /**
   * Phase 1: Determine current season year + next season's first game timestamp
   * 
   * Logic:
   *   1. Fetch calendar for current year
   *   2. If in season: use this year, fetch first game of *next* year to store as boundary
   *   3. If NOT in season: fetch calendar for last year, check if in that season window
   *      - If in last year's window: use last year (postseason), fetch next year's first game
   *      - If not: this is offseason between seasons, fetch next year's first game as boundary
   */
  function determineSeasonAndBoundary(apiKey, callback) {
    var now = new Date();
    var currentYear = now.getFullYear();

    console.log('Phase 1: Determine season (current year: ' + currentYear + ')');

    // Try current year first
    fetchCalendar(currentYear, apiKey, function(calendarResult) {

      // 1. Define the next step as a helper function
      const fetchNextSeasonBoundary = function() {
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

        // 2. Execute here if current year is valid
        fetchNextSeasonBoundary();
      }
      else {
        // Try last year
        fetchCalendar(currentYear - 1, apiKey, function(lastYearResult) {
          if (lastYearResult.isInSeason || lastYearResult.postSeason) {
            console.log('In postseason: using year ' + (currentYear - 1));
            cache.currentYear = currentYear - 1;
            cache.seasonDates[0] = lastYearResult.startDate;
            cache.seasonDates[1] = lastYearResult.endDate;
            cache.weekDates = lastYearResult.weekDates;

            // 3. Execute here if last year is valid
            fetchNextSeasonBoundary();
          } else {
            // Offseason: use next year, fetch its first game
            console.error('No Schedules Found');
            callback([]);
            return; // Stops execution, fetchNextSeasonBoundary is never called
          }
        });
      }
    });
  }

  /**
   * Helper: Fetch the first game (by date) of a given year's season
   */
  function fetchFirstGameOfYear(year, apiKey, callback) {
    fetchGames(year, 1, false, apiKey, function(games) {
      if (games.length > 0) {
        // games are already sorted by startDate from fetchGames
        callback(games[0]);
      } else {
        callback(null);
      }
    });
  }

  /**
 * Helper: given the cached season info, figure out which (year, week)
 * we should be fetching, and whether we're in the offseason.
 * Returns: { year, week, offseason }
 */
  function determineCurrentWeek(cache) {
    var now = new Date();
    var nowTs = Math.floor(now.getTime() / 1000);

    var TWO_WEEKS_SECONDS = 14 * 24 * 60 * 60;

    // Branch 1: within 2 weeks of next season's kickoff -> jump to new season, week 0
    if (cache.nextSeasonFirstGameTs && nowTs >= (cache.nextSeasonFirstGameTs - TWO_WEEKS_SECONDS)) {
      console.log('Within 2 weeks of next season kickoff - using year ' + (cache.currentYear + 1) + ', week 0');
      return {
        year: cache.currentYear + 1,
        week: 0,
        offseason: false
      };
    }

    // Branch 2: currently inside the active season window
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
      // Fallback: inside seasonDates but didn't land inside any single week's
      // start/end (gaps between weeks happen) - use the last known week.
      var fallbackEntry = cache.weekDates[cache.weekDates.length - 1];
      console.log('In season but between week boundaries - using last known week ' + fallbackEntry[0]);
      return {
        year: cache.currentYear,
        week: fallbackEntry[0],
        offseason: false
      };
    }

    // Branch 3: not in season, not near next season -> offseason
    var lastEntry = cache.weekDates[cache.weekDates.length - 1];
    console.log('Offseason - using last week of ' + cache.currentYear + ': week ' + lastEntry[0]);
    return {
      year: cache.currentYear,
      week: lastEntry[0],
      offseason: true
    };
  }

  /**
   * Public API
   */
  return {
    cache: cache,

    /**
     * Full workflow: determine season + boundary only. Calendar data
     * (year, next season kickoff, season/week date ranges) is all the
     * watch needs from a full sync - games/records/rankings are fetched
     * separately by syncLightCFBD, on its own trigger, for whatever the
     * current week turns out to be.
     * Call on app launch and periodically (e.g., daily).
     */
    syncFullCFBD: function(apiKey, callback) {
      console.log('=== CFBD Full Sync Start (calendar only) ===');

      determineSeasonAndBoundary(apiKey, function(year, nextSeasonTs, seasonDates, weekDates) {
        console.log('=== CFBD Season Boundary Determined ===');
        callback({
          year: year,
          nextSeasonFirstGameTs: nextSeasonTs,
          seasonDates: seasonDates,
          weekDates: weekDates
        });
      });
    },

    /**
   * Lighter refresh: uses cached season info from syncFullCFBD to determine
   * the correct year/week, then fetches games+records+rankings (or just
   * records+rankings if we're in the offseason).
   *
   * determineCurrentWeek needs cache.currentYear/seasonDates/weekDates to
   * already be populated (normally true after a prior syncFullCFBD call in
   * this same JS session). If the JS worker restarted and light sync fires
   * first - which can happen, since the watch decides to sync based on its
   * own persisted timestamps, not on what this JS session has done - that
   * cache would be empty and determineCurrentWeek would throw. So: if
   * cache.currentYear is unset, run determineSeasonAndBoundary first to
   * populate it, then proceed exactly as before.
   */
    syncLightCFBD: function(apiKey, callback) {
      if (cache.currentYear === null) {
        console.log('Light sync: cache empty (no full sync this session yet) - determining season first');
        determineSeasonAndBoundary(apiKey, function() {
          doLightSync(apiKey, callback);
        });
        return;
      }
      doLightSync(apiKey, callback);
    }
  };

  function doLightSync(apiKey, callback) {
    console.log('=== CFBD Light Sync Start ===');
    var target = determineCurrentWeek(cache);
    console.log('Light sync: year ' + target.year + ', week ' + target.week +
                (target.offseason ? ' (offseason)' : ''));

    var results = {
      year: target.year,
      week: target.week,
      offseason: target.offseason,
      games: [],
      records: [],
      rankings: []
    };

    //var expected = target.offseason ? 2 : 3; // records+rankings, or +games
    var expected = 3;
    var completed = 0;

    function onComplete() {
      completed++;
      if (completed === expected) callback(results);
    }

    fetchRecords(target.year, apiKey, function(data) {
      results.records = data;
      onComplete();
    });

    setTimeout(function() {
      //fetchRankings(target.year, target.week, target.offseason, apiKey, function(data) {
      fetchRankings(target.year, 13, false, apiKey, function(data) {
        results.rankings = data;
        onComplete();
      });
    }, constants.BATCH_DELAY);

    // week 13 is hardcoded for now (offseason testing, so real "current
    // week" games don't exist yet) - swap the two lines below (comment
    // the 13 one, uncomment target.week one) once testing is done and the
    // season's actual current week should be used instead.
    //if (!target.offseason) {
      setTimeout(function() {
        //fetchGames(target.year, target.week, target.offseason, apiKey, function(data) {
        fetchGames(target.year, 13, false, apiKey, function(data) {
          results.games = data;
          onComplete();
        });
      }, constants.BATCH_DELAY * 2);
    //}
  }
})();

module.exports = cfbd;