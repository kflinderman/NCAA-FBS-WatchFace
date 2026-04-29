// teams.c
#include "teams.h" // Include the header to get the 'Team' type

#if defined(PBL_COLOR)
const Team TEAMS[] = {
  { .logo_res_id = RESOURCE_ID_GAMECOCK, .color = GColorDarkCandyAppleRedARGB8, .name = "Gamecock" },
  { .logo_res_id = RESOURCE_ID_CLEMSON, .color = GColorOrangeARGB8, .name = "Clemson" },
};
#else
const Team TEAMS[] = {
  { .logo_res_id = RESOURCE_ID_GAMECOCK, .color = GColorBlackARGB8, .name = "Gamecock" },
  { .logo_res_id = RESOURCE_ID_CLEMSON, .color = GColorBlackARGB8, .name = "Clemson" },
};
#endif

// Define the count based on the array
const size_t TEAMS_COUNT = sizeof(TEAMS) / sizeof(TEAMS[0]);