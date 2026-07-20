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


API_Info API_DATA[] = {
  { .name = "Clemson", .id = 0 },
  { .name = "South Carolina", .id = 1 },
};

// Number of entries in API_DATA — sized for the test-bed roster, not the
// full 154-team NUM_TEAMS. Use this (not NUM_TEAMS) when iterating/parsing
// so the small test roster doesn't require touching parser code as the
// real roster grows.
const size_t API_DATA_COUNT = sizeof(API_DATA) / sizeof(API_DATA[0]);

// Define the count based on the array
const size_t TEAMS_COUNT = sizeof(TEAMS) / sizeof(TEAMS[0]);