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