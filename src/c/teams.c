// teams.c
#include "teams.h" // Include the header to get the 'Team' type

#if defined(PBL_COLOR)
const Team TEAMS[] = {
  { .logo_res_id = RESOURCE_ID_CLEMSON, .color = GColorOrangeARGB8, .icon_color = GColorBlackARGB8, .rival = 1, .name = "Clemson" },
  { .logo_res_id = RESOURCE_ID_GAMECOCK, .color = GColorDarkCandyAppleRedARGB8, .icon_color = GColorWhiteARGB8, .rival = 0, .name = "South Carolina" },
};
#else
const Team TEAMS[] = {
  { .logo_res_id = RESOURCE_ID_CLEMSON, .color = GColorBlackARGB8, .icon_color = GColorBlackARGB8, .rival = 1, .name = "Clemson" },
  { .logo_res_id = RESOURCE_ID_GAMECOCK, .color = GColorBlackARGB8, .icon_color = GColorWhiteARGB8, .rival = 0, .name = "South Carolina" },
};
#endif


API_Info api_info[] = {
  { .name = "Clemson", .id = 0 },
  { .name = "South Carolina", .id = 1 },
};

// Define the count based on the array
const size_t TEAMS_COUNT = sizeof(TEAMS) / sizeof(TEAMS[0]);