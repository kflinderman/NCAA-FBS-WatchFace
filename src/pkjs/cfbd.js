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
  function fetchGames(year, week, apiKey, callback) {
    var url = constants.API_BASE + '/games?year=' + year + '&week=' + week + '&classification=fbs';
    //games?year=2025&week=1&classification=fbs
    
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

      console.log('Fetched ' + data.length + ' games for week ' + week);
      callback(data);
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

  /**
   * GET /records for all teams in a year
   * Returns: [ { team, year, total: { wins, losses }, conf: { wins, losses }, postseason: { ... } }, ... ]
   */
  function fetchRecords(year, apiKey, callback) {
    var url = constants.API_BASE + '/records?year=' + year;
    
    xhrAuth(url, apiKey, function(data) {
      if (!data || !Array.isArray(data)) {
        console.log('No records for year ' + year);
        callback([]);
        return;
      }
      console.log('Fetched records for ' + data.length + ' teams');
      cache.records = data;
      callback(data);
    }, function(status) {
      console.log('fetchRecords failed: ' + status);
      callback([]);
    });
  }

  /**
   * GET /rankings for a year (current/latest poll)
   * Returns: [ { year, week, poll, ranks: [ { rank, team, ... }, ... ] }, ... ]
   */
  function fetchRankings(year, apiKey, callback) {
    var url = constants.API_BASE + '/rankings?year=' + year;
    
    xhrAuth(url, apiKey, function(data) {
      if (!data || !Array.isArray(data)) {
        console.log('No rankings for year ' + year);
        callback([]);
        return;
      }
      console.log('Fetched ' + data.length + ' ranking polls');
      cache.rankings = data;
      callback(data);
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
      if (calendarResult.isInSeason) {
        console.log('Using year ' + currentYear);
        cache.currentYear = currentYear;
        cache.seasonDates[0] = calendarResult.startDate;
        cache.seasonDates[1] = calendarResult.endDate;
        cache.weekDates = calendarResult.weekDates;
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
          } else {
            // Offseason: use next year, fetch its first game
            console.error('No Schedules Found');
            callback([]);
            return;
          }
        });
      }
      // Fetch current year's first game as boundary
      fetchFirstGameOfYear(cache.currentYear + 1, apiKey, function(firstGame) {
        if (firstGame && firstGame.startDate) {
          cache.nextSeasonFirstGameTs = Math.floor(new Date(firstGame.startDate).getTime() / 1000);
        }
        callback(cache.currentYear, cache.nextSeasonFirstGameTs, cache.seasonDates, cache.weekDates);
      });
    });
  }

  /**
   * Helper: Fetch the first game (by date) of a given year's season
   */
  function fetchFirstGameOfYear(year, apiKey, callback) {
    fetchGames(year, 1, apiKey, function(games) {
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
   * Phase 2: Batch fetch all CFBD data (games, records, rankings) for a year
   * 
   * Call this periodically (e.g., daily or after user triggers a refresh)
   * Returns all three datasets aggregated
   */
  function batchFetchSeasonData(year, apiKey, callback) {
    console.log('Phase 2: Batch fetch season data for year ' + year);

    var results = {
      games: [],
      records: [],
      rankings: []
    };
    var completed = 0;

    function onComplete() {
      completed++;
      if (completed === 3) {
        console.log('All batch data fetched');
        callback(results);
      }
    }

    // Stagger the requests slightly to avoid connection pool issues
    setTimeout(function() {
      fetchAllGamesForYear(year, apiKey, function(data) {
        results.games = data;
        onComplete();
      });
    }, 0);

    setTimeout(function() {
      fetchRecords(year, apiKey, function(data) {
        results.records = data;
        onComplete();
      });
    }, constants.BATCH_DELAY);

    setTimeout(function() {
      fetchRankings(year, apiKey, function(data) {
        results.rankings = data;
        onComplete();
      });
    }, constants.BATCH_DELAY * 2);
  }

  /**
   * Public API
   */
  return {
    cache: cache,

    /**
     * Full workflow: determine season + fetch all data
     * Call on app launch and periodically (e.g., daily)
     */
    syncFullCFBD: function(apiKey, callback) {
      console.log('=== CFBD Full Sync Start ===');

      determineSeasonAndBoundary(apiKey, function(year, nextSeasonTs, seasonDates, weekDates) {
        console.log('=== CFBD Season Boundary Determined ===');

        // cache is already populated by determineSeasonAndBoundary at this point
        // (cache.currentYear, cache.nextSeasonFirstGameTs, cache.seasonDates, cache.weekDates)
        // so syncLightCFBD can read off it directly.
        this.syncLightCFBD(apiKey, function(lightResults) {
          console.log('=== CFBD Full Sync Complete ===');
          callback({
            year: year,
            nextSeasonFirstGameTs: nextSeasonTs,
            seasonDates: seasonDates,
            weekDates: weekDates,
            games: lightResults.games,
            records: lightResults.records,
            rankings: lightResults.rankings,
            week: lightResults.week,
            offseason: lightResults.offseason
          });
        });
      }.bind(this));
    },

    /**
   * Lighter refresh: uses cached season info from syncFullCFBD to determine
   * the correct year/week, then fetches games+records+rankings (or just
   * records+rankings if we're in the offseason).
   */
    syncLightCFBD: function(apiKey, callback) {
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

      var expected = target.offseason ? 2 : 3; // records+rankings, or +games
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
        fetchRankings(target.year, apiKey, function(data) {
          results.rankings = data;
          onComplete();
        });
      }, constants.BATCH_DELAY);

      if (!target.offseason) {
        setTimeout(function() {
          fetchGames(target.year, target.week, apiKey, function(data) {
            results.games = data;
            onComplete();
          });
        }, constants.BATCH_DELAY * 2);
      }
    }
  };
})();

module.exports = cfbd;