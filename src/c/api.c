#include <pebble.h>
#include "api.h"

/*
GET /calendar current year
if within any dates
GET /games current week
else
GET /calendar last year
if within dates
GET /games current week
else
GET /games postseason week 1


Champ stuff
GET /records
if total.wins > 6
winning season
if last regular season game contains Championship + week
Conf Champ
if postseason.games = 1 && postseason.wins = 1 (need to figure out other playoff teams not getting this)
Bowl win
if postseason.games > 1 && postseason.loses != 1 (need to figure out other playoff teams not getting this)
Champion


Do I make an API indicator? Like the battery level, but of # of calls in a month? Not sure how I track this
*/

#include <pebble.h>
#include "api.h"
#include "globals.h"

void api_request_score(void) {
  // Respect the same on/off + quiet-time gating as weather, using the
  // score-specific settings fields that already exist in ClaySettings.
  if (!settings.api || !settings.scoreDisplayBool) {
    return;
  }
  if (settings.api_key[0] == '\0') {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Score request skipped: no API key configured");
    return;
  }

  // Which team to look up: the opponent if the user has one selected,
  // otherwise the favorite team. opponentSelect/customOpponent let JS
  // resolve a non-favorite opponent; FavoriteTeam otherwise.
  uint16_t teamIndex = settings.FavoriteTeam;
  if (settings.opponentBool && settings.opponentSelect == 2) {
    teamIndex = settings.customOpponent;
  } else if (settings.opponentBool) {
    teamIndex = settings.BeatTeam;
  }

  APP_LOG(APP_LOG_LEVEL_INFO, "Score Send");
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_uint8(iter, MESSAGE_KEY_REQUEST_SCORE, 1);
  dict_write_cstring(iter, MESSAGE_KEY_api_key, settings.api_key);
  dict_write_uint16(iter, MESSAGE_KEY_ScoreTeamIndex, teamIndex);
  app_message_outbox_send();
}

void api_score_callback(DictionaryIterator *iterator, void *context) {
  bool score_changed = false;

  Tuple *home_team_tuple = dict_find(iterator, MESSAGE_KEY_ScoreHomeTeam);
  Tuple *away_team_tuple = dict_find(iterator, MESSAGE_KEY_ScoreAwayTeam);
  Tuple *home_points_tuple = dict_find(iterator, MESSAGE_KEY_ScoreHomePoints);
  Tuple *away_points_tuple = dict_find(iterator, MESSAGE_KEY_ScoreAwayPoints);
  Tuple *completed_tuple = dict_find(iterator, MESSAGE_KEY_ScoreCompleted);

  if (home_team_tuple) {
    snprintf(scoreHomeTeam, sizeof(scoreHomeTeam), "%s", home_team_tuple->value->cstring);
    score_changed = true;
  }
  if (away_team_tuple) {
    snprintf(scoreAwayTeam, sizeof(scoreAwayTeam), "%s", away_team_tuple->value->cstring);
    score_changed = true;
  }
  if (home_points_tuple) {
    scoreHomePoints = home_points_tuple->value->int16;
    score_changed = true;
  }
  if (away_points_tuple) {
    scoreAwayPoints = away_points_tuple->value->int16;
    score_changed = true;
  }
  if (completed_tuple) {
    scoreCompleted = completed_tuple->value->int8 != 0;
    score_changed = true;
  }

  if (score_changed) {
    scoreValid = true;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Score updated: %s %d - %d %s",
      scoreAwayTeam, scoreAwayPoints, scoreHomePoints, scoreHomeTeam);
    // Drawing/display hookup (e.g. score_draw()/score_update()) is a
    // separate step once the score UI layer exists — not added here
    // to avoid guessing at layout code that doesn't exist yet.
  }
}