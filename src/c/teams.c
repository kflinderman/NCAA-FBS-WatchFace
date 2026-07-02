// teams.c
#include "teams.h" // Include the header to get the 'Team' type

#if defined(PBL_COLOR)
const Team TEAMS[] = {
  { .logo_res_id = RESOURCE_ID_CLEMSON, .color = GColorOrangeARGB8, .icon_color = GColorWhiteARGB8 },
  { .logo_res_id = RESOURCE_ID_GAMECOCK, .color = GColorDarkCandyAppleRedARGB8, .icon_color = GColorWhiteARGB8 },
};
#else
const Team TEAMS[] = {
  { .logo_res_id = RESOURCE_ID_CLEMSON, .color = GColorBlackARGB8, .icon_color = GColorWhiteARGB8 },
  { .logo_res_id = RESOURCE_ID_GAMECOCK, .color = GColorBlackARGB8, .icon_color = GColorWhiteARGB8 },
};
#endif

// Define the count based on the array
const size_t TEAMS_COUNT = sizeof(TEAMS) / sizeof(TEAMS[0]);