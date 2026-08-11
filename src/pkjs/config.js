// FYI https://github.com/pebble-dev/clay#showing-items-for-specific-platforms-and-features
module.exports = [
  {
    "type": "heading",
    "defaultValue": "NCAA FBS Watchface Settings"
  },
  {
    "type": "text",
    "defaultValue": "Show your passion."
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Display"
      },
      {
	      "type": "radiogroup",
	      "messageKey": "DisplayTeam",
	      "label": "What would you like your main display to be?",
		    "defaultValue": "0",
	      "options": [
	  	    { 
		        "label": "Favorite Team", 
		        "value": "0"
		      },
		      { 
		        "label": "Team You Want to Beat", 
		        "value": "30"
		      }
		    ]
	    },
      {
        "type": "toggle",
        "messageKey": "bagBool",
        "label": "Bag",
        "defaultValue": false,
        "description": "Are you embarrased by your team?"
      },
	    {
		    "type": "select",
		    "messageKey": "FavoriteTeam",
		    "defaultValue": 108,
		    "label": "Favorite Team",
		    "options": [
          { "label": "ACC", "value": 0 },
          { "label": "Air Force", "value": 1 },
          { "label": "Akron", "value": 2 },
          { "label": "Alabama", "value": 3 },
          { "label": "American", "value": 4 },
          { "label": "App State", "value": 5 },
          { "label": "Arizona", "value": 6 },
          { "label": "Arizona State", "value": 7 },
          { "label": "Arkansas", "value": 8 },
          { "label": "Arkansas State", "value": 9 },
          { "label": "Army", "value": 10 },
          { "label": "Auburn", "value": 11 },
          { "label": "B1G", "value": 12 },
          { "label": "Ball State", "value": 13 },
          { "label": "Baylor", "value": 14 },
          { "label": "BIG 12", "value": 15 },
          { "label": "Boise State", "value": 16 },
          { "label": "Boston College", "value": 17 },
          { "label": "Bowling Green", "value": 18 },
          { "label": "Buffalo", "value": 19 },
          { "label": "BYU", "value": 20 },
          { "label": "California", "value": 21 },
          { "label": "Central Florida", "value": 126 },
          { "label": "Central Michigan", "value": 22 },
          { "label": "CFB", "value": 23 },
          { "label": "Chaos", "value": 24 },
          { "label": "Charlotte", "value": 133 },
          { "label": "Cincinnati", "value": 25 },
          { "label": "Clemson", "value": 26 },
          { "label": "Coastal Carolina", "value": 27 },
          { "label": "Colorado", "value": 28 },
          { "label": "Colorado State", "value": 29 },
          { "label": "Connecticut", "value": 128 },
          { "label": "CUSA", "value": 30 },
          { "label": "Deleware", "value": 31 },
          { "label": "Duke", "value": 32 },
          { "label": "Eastern Michigan", "value": 33 },
          { "label": "East Carolina", "value": 34 },
          { "label": "FAU", "value": 35 },
          { "label": "Florida International", "value": 36 },
          { "label": "Florida", "value": 37 },
          { "label": "Fresno State", "value": 38 },
          { "label": "Florida State", "value": 39 },
          { "label": "Georgia", "value": 129 },
          { "label": "Georgia Southern", "value": 40 },
          { "label": "Georgia State", "value": 41 },
          { "label": "Georgia Tech", "value": 42 },
          { "label": "Hawaii", "value": 43 },
          { "label": "Houston", "value": 44 },
          { "label": "Illinois", "value": 45 },
          { "label": "Independent", "value": 46 },
          { "label": "Indiana", "value": 47 },
          { "label": "Iowa", "value": 48 },
          { "label": "Iowa State", "value": 49 },
          { "label": "Jacksonville State", "value": 50 },
          { "label": "James Maddison", "value": 51 },
          { "label": "Kansas", "value": 52 },
          { "label": "Kennesaw State", "value": 53 },
          { "label": "Kent State", "value": 54 },
          { "label": "Kentucky", "value": 55 },
          { "label": "Kansas State", "value": 56 },
          { "label": "LA Tech", "value": 57 },
          { "label": "Liberty", "value": 58 },
          { "label": "Louisville", "value": 59 },
          { "label": "LSU", "value": 60 },
          { "label": "MAC", "value": 61 },
          { "label": "Marshall", "value": 62 },
          { "label": "Maryland", "value": 63 },
          { "label": "Massachusetts", "value": 132 },
          { "label": "Memphis", "value": 64 },
          { "label": "Meteor", "value": 65 },
          { "label": "Miami Ohio", "value": 66 },
          { "label": "Miami", "value": 67 },
          { "label": "Michigan", "value": 68 },
          { "label": "Michigan State", "value": 69 },
          { "label": "Middle Tennessee State", "value": 70 },
          { "label": "Minnesota", "value": 71 },
          { "label": "Missouri State", "value": 72 },
          { "label": "Mississippi State", "value": 73 },
          { "label": "Missouri", "value": 74 },
          { "label": "Mountain West", "value": 75 },
          { "label": "Navy", "value": 76 },
          { "label": "NCAA", "value": 77 },
          { "label": "North Carolina State", "value": 78 },
          { "label": "North Dakota State", "value": 79 },
          { "label": "Nebraska", "value": 80 },
          { "label": "Nevada", "value": 81 },
          { "label": "New Mexico", "value": 82 },
          { "label": "New Mexico State", "value": 83 },
          { "label": "NIU", "value": 84 },
          { "label": "North Carolina", "value": 134 },
          { "label": "North Texas", "value": 136 },
          { "label": "Northwestern", "value": 85 },
          { "label": "Notre Dame", "value": 86 },
          { "label": "Old Dominion", "value": 87 },
          { "label": "Ohio", "value": 88 },
          { "label": "Oklahoma State", "value": 89 },
          { "label": "Ole Miss", "value": 90 },
          { "label": "Oregon", "value": 91 },
          { "label": "Oregon State", "value": 92 },
          { "label": "Ohio State", "value": 93 },
          { "label": "Oklahoma", "value": 94 },
          { "label": "PAC 12", "value": 95 },
          { "label": "Penn State", "value": 96 },
          { "label": "Pitt", "value": 97 },
          { "label": "Purdue", "value": 98 },
          { "label": "Rice", "value": 99 },
          { "label": "Rutgers", "value": 100 },
          { "label": "Sacramento State", "value": 101 },
          { "label": "SHSU", "value": 102 },
          { "label": "SDSU", "value": 103 },
          { "label": "SEC", "value": 104 },
          { "label": "SJSU", "value": 105 },
          { "label": "SMU", "value": 106 },
          { "label": "South Alabama", "value": 107 },
          { "label": "South Carolina", "value": 108 },
          { "label": "South Florida", "value": 137 },
          { "label": "Southern Cal", "value": 109 },
          { "label": "Southern Miss", "value": 110 },
          { "label": "Stanford", "value": 111 },
          { "label": "Sun Belt", "value": 112 },
          { "label": "Syracuse", "value": 113 },
          { "label": "TCU", "value": 114 },
          { "label": "Temple", "value": 115 },
          { "label": "Tennessee", "value": 116 },
          { "label": "Texas", "value": 117 },
          { "label": "Texas A&M", "value": 118 },
          { "label": "Texas State", "value": 119 },
          { "label": "Texas Tech", "value": 120 },
          { "label": "Toledo", "value": 121 },
          { "label": "Troy", "value": 122 },
          { "label": "Tulane", "value": 123 },
          { "label": "Tulsa", "value": 124 },
          { "label": "UAB", "value": 125 },
          { "label": "UCLA", "value": 127 },
          { "label": "ULL", "value": 130 },
          { "label": "ULM", "value": 131 },
          { "label": "UNLV", "value": 135 },
          { "label": "Utah", "value": 138 },
          { "label": "Utah State", "value": 139 },
          { "label": "UTEP", "value": 140 },
          { "label": "UTSA", "value": 141 },
          { "label": "Virginia", "value": 142 },
          { "label": "Vanderbilt", "value": 143 },
          { "label": "Virginia Tech", "value": 144 },
          { "label": "Wake Forest", "value": 145 },
          { "label": "Washington", "value": 146 },
          { "label": "Washington State", "value": 147 },
          { "label": "Western Michigan", "value": 148 },
          { "label": "Wisconsin", "value": 149 },
          { "label": "Western Kentucky", "value": 150 },
          { "label": "West Virginia", "value": 151 },
          { "label": "Wyoming", "value": 152 }
        ]
	    },
      {
        "type": "select",
        "messageKey": "hardcodeRivalBool",
        "label": "Who to beat?",
        "defaultValue": 0,
        "options": [
		      { "label": "Custom", "value": 0 },
		      { "label": "Rival", "value": 1 },
		      { "label": "Schedule - API ONLY", "value": 2 }
        ]
      },
	    {
		    "type": "select",
		    "messageKey": "BeatTeam",
		    "defaultValue": 26,
		    "label": "Beat Team",
		    "options": [
          { "label": "ACC", "value": 0 },
          { "label": "Air Force", "value": 1 },
          { "label": "Akron", "value": 2 },
          { "label": "Alabama", "value": 3 },
          { "label": "American", "value": 4 },
          { "label": "App State", "value": 5 },
          { "label": "Arizona", "value": 6 },
          { "label": "Arizona State", "value": 7 },
          { "label": "Arkansas", "value": 8 },
          { "label": "Arkansas State", "value": 9 },
          { "label": "Army", "value": 10 },
          { "label": "Auburn", "value": 11 },
          { "label": "B1G", "value": 12 },
          { "label": "Ball State", "value": 13 },
          { "label": "Baylor", "value": 14 },
          { "label": "BIG 12", "value": 15 },
          { "label": "Boise State", "value": 16 },
          { "label": "Boston College", "value": 17 },
          { "label": "Bowling Green", "value": 18 },
          { "label": "Buffalo", "value": 19 },
          { "label": "BYU", "value": 20 },
          { "label": "California", "value": 21 },
          { "label": "Central Florida", "value": 126 },
          { "label": "Central Michigan", "value": 22 },
          { "label": "CFB", "value": 23 },
          { "label": "Chaos", "value": 24 },
          { "label": "Charlotte", "value": 133 },
          { "label": "Cincinnati", "value": 25 },
          { "label": "Clemson", "value": 26 },
          { "label": "Coastal Carolina", "value": 27 },
          { "label": "Colorado", "value": 28 },
          { "label": "Colorado State", "value": 29 },
          { "label": "Connecticut", "value": 128 },
          { "label": "CUSA", "value": 30 },
          { "label": "Deleware", "value": 31 },
          { "label": "Duke", "value": 32 },
          { "label": "Eastern Michigan", "value": 33 },
          { "label": "East Carolina", "value": 34 },
          { "label": "FAU", "value": 35 },
          { "label": "Florida International", "value": 36 },
          { "label": "Florida", "value": 37 },
          { "label": "Fresno State", "value": 38 },
          { "label": "Florida State", "value": 39 },
          { "label": "Georgia", "value": 129 },
          { "label": "Georgia Southern", "value": 40 },
          { "label": "Georgia State", "value": 41 },
          { "label": "Georgia Tech", "value": 42 },
          { "label": "Hawaii", "value": 43 },
          { "label": "Houston", "value": 44 },
          { "label": "Illinois", "value": 45 },
          { "label": "Independent", "value": 46 },
          { "label": "Indiana", "value": 47 },
          { "label": "Iowa", "value": 48 },
          { "label": "Iowa State", "value": 49 },
          { "label": "Jacksonville State", "value": 50 },
          { "label": "James Maddison", "value": 51 },
          { "label": "Kansas", "value": 52 },
          { "label": "Kennesaw State", "value": 53 },
          { "label": "Kent State", "value": 54 },
          { "label": "Kentucky", "value": 55 },
          { "label": "Kansas State", "value": 56 },
          { "label": "LA Tech", "value": 57 },
          { "label": "Liberty", "value": 58 },
          { "label": "Louisville", "value": 59 },
          { "label": "LSU", "value": 60 },
          { "label": "MAC", "value": 61 },
          { "label": "Marshall", "value": 62 },
          { "label": "Maryland", "value": 63 },
          { "label": "Massachusetts", "value": 132 },
          { "label": "Memphis", "value": 64 },
          { "label": "Meteor", "value": 65 },
          { "label": "Miami Ohio", "value": 66 },
          { "label": "Miami", "value": 67 },
          { "label": "Michigan", "value": 68 },
          { "label": "Michigan State", "value": 69 },
          { "label": "Middle Tennessee State", "value": 70 },
          { "label": "Minnesota", "value": 71 },
          { "label": "Missouri State", "value": 72 },
          { "label": "Mississippi State", "value": 73 },
          { "label": "Missouri", "value": 74 },
          { "label": "Mountain West", "value": 75 },
          { "label": "Navy", "value": 76 },
          { "label": "NCAA", "value": 77 },
          { "label": "North Carolina State", "value": 78 },
          { "label": "North Dakota State", "value": 79 },
          { "label": "Nebraska", "value": 80 },
          { "label": "Nevada", "value": 81 },
          { "label": "New Mexico", "value": 82 },
          { "label": "New Mexico State", "value": 83 },
          { "label": "NIU", "value": 84 },
          { "label": "North Carolina", "value": 134 },
          { "label": "North Texas", "value": 136 },
          { "label": "Northwestern", "value": 85 },
          { "label": "Notre Dame", "value": 86 },
          { "label": "Old Dominion", "value": 87 },
          { "label": "Ohio", "value": 88 },
          { "label": "Oklahoma State", "value": 89 },
          { "label": "Ole Miss", "value": 90 },
          { "label": "Oregon", "value": 91 },
          { "label": "Oregon State", "value": 92 },
          { "label": "Ohio State", "value": 93 },
          { "label": "Oklahoma", "value": 94 },
          { "label": "PAC 12", "value": 95 },
          { "label": "Penn State", "value": 96 },
          { "label": "Pitt", "value": 97 },
          { "label": "Purdue", "value": 98 },
          { "label": "Rice", "value": 99 },
          { "label": "Rutgers", "value": 100 },
          { "label": "Sacramento State", "value": 101 },
          { "label": "SHSU", "value": 102 },
          { "label": "SDSU", "value": 103 },
          { "label": "SEC", "value": 104 },
          { "label": "SJSU", "value": 105 },
          { "label": "SMU", "value": 106 },
          { "label": "South Alabama", "value": 107 },
          { "label": "South Carolina", "value": 108 },
          { "label": "South Florida", "value": 137 },
          { "label": "Southern Cal", "value": 109 },
          { "label": "Southern Miss", "value": 110 },
          { "label": "Stanford", "value": 111 },
          { "label": "Sun Belt", "value": 112 },
          { "label": "Syracuse", "value": 113 },
          { "label": "TCU", "value": 114 },
          { "label": "Temple", "value": 115 },
          { "label": "Tennessee", "value": 116 },
          { "label": "Texas", "value": 117 },
          { "label": "Texas A&M", "value": 118 },
          { "label": "Texas State", "value": 119 },
          { "label": "Texas Tech", "value": 120 },
          { "label": "Toledo", "value": 121 },
          { "label": "Troy", "value": 122 },
          { "label": "Tulane", "value": 123 },
          { "label": "Tulsa", "value": 124 },
          { "label": "UAB", "value": 125 },
          { "label": "UCLA", "value": 127 },
          { "label": "ULL", "value": 130 },
          { "label": "ULM", "value": 131 },
          { "label": "UNLV", "value": 135 },
          { "label": "Utah", "value": 138 },
          { "label": "Utah State", "value": 139 },
          { "label": "UTEP", "value": 140 },
          { "label": "UTSA", "value": 141 },
          { "label": "Virginia", "value": 142 },
          { "label": "Vanderbilt", "value": 143 },
          { "label": "Virginia Tech", "value": 144 },
          { "label": "Wake Forest", "value": 145 },
          { "label": "Washington", "value": 146 },
          { "label": "Washington State", "value": 147 },
          { "label": "Western Michigan", "value": 148 },
          { "label": "Wisconsin", "value": 149 },
          { "label": "Western Kentucky", "value": 150 },
          { "label": "West Virginia", "value": 151 },
          { "label": "Wyoming", "value": 152 }
        ]
	    },
      {
        "type": "select",
        "defaultValue": 0,
        "label": "Bye Opponent",
        "messageKey": "opponentSelect",
        "description": "What to display on a bye week, this team will also be used when API data is unavialable",
      		"options": [
      		  {
      		    "label": "NCAA",
      			  "value": 0
      		  },
      		  {
      		    "label": "Rival",
      			  "value": 1
      		  },
      		  {
              "label": "Custom",
              "value": 2
            }
          ]
      },
      {
        "type": "select",
        "defaultValue": 26,
        "label": "Custom Team",
        "messageKey": "customOpponent",
        "options": [
          { "label": "ACC", "value": 0 },
          { "label": "Air Force", "value": 1 },
          { "label": "Akron", "value": 2 },
          { "label": "Alabama", "value": 3 },
          { "label": "American", "value": 4 },
          { "label": "App State", "value": 5 },
          { "label": "Arizona", "value": 6 },
          { "label": "Arizona State", "value": 7 },
          { "label": "Arkansas", "value": 8 },
          { "label": "Arkansas State", "value": 9 },
          { "label": "Army", "value": 10 },
          { "label": "Auburn", "value": 11 },
          { "label": "B1G", "value": 12 },
          { "label": "Ball State", "value": 13 },
          { "label": "Baylor", "value": 14 },
          { "label": "BIG 12", "value": 15 },
          { "label": "Boise State", "value": 16 },
          { "label": "Boston College", "value": 17 },
          { "label": "Bowling Green", "value": 18 },
          { "label": "Buffalo", "value": 19 },
          { "label": "BYU", "value": 20 },
          { "label": "California", "value": 21 },
          { "label": "Central Florida", "value": 126 },
          { "label": "Central Michigan", "value": 22 },
          { "label": "CFB", "value": 23 },
          { "label": "Chaos", "value": 24 },
          { "label": "Charlotte", "value": 133 },
          { "label": "Cincinnati", "value": 25 },
          { "label": "Clemson", "value": 26 },
          { "label": "Coastal Carolina", "value": 27 },
          { "label": "Colorado", "value": 28 },
          { "label": "Colorado State", "value": 29 },
          { "label": "Connecticut", "value": 128 },
          { "label": "CUSA", "value": 30 },
          { "label": "Deleware", "value": 31 },
          { "label": "Duke", "value": 32 },
          { "label": "Eastern Michigan", "value": 33 },
          { "label": "East Carolina", "value": 34 },
          { "label": "FAU", "value": 35 },
          { "label": "Florida International", "value": 36 },
          { "label": "Florida", "value": 37 },
          { "label": "Fresno State", "value": 38 },
          { "label": "Florida State", "value": 39 },
          { "label": "Georgia", "value": 129 },
          { "label": "Georgia Southern", "value": 40 },
          { "label": "Georgia State", "value": 41 },
          { "label": "Georgia Tech", "value": 42 },
          { "label": "Hawaii", "value": 43 },
          { "label": "Houston", "value": 44 },
          { "label": "Illinois", "value": 45 },
          { "label": "Independent", "value": 46 },
          { "label": "Indiana", "value": 47 },
          { "label": "Iowa", "value": 48 },
          { "label": "Iowa State", "value": 49 },
          { "label": "Jacksonville State", "value": 50 },
          { "label": "James Maddison", "value": 51 },
          { "label": "Kansas", "value": 52 },
          { "label": "Kennesaw State", "value": 53 },
          { "label": "Kent State", "value": 54 },
          { "label": "Kentucky", "value": 55 },
          { "label": "Kansas State", "value": 56 },
          { "label": "LA Tech", "value": 57 },
          { "label": "Liberty", "value": 58 },
          { "label": "Louisville", "value": 59 },
          { "label": "LSU", "value": 60 },
          { "label": "MAC", "value": 61 },
          { "label": "Marshall", "value": 62 },
          { "label": "Maryland", "value": 63 },
          { "label": "Massachusetts", "value": 132 },
          { "label": "Memphis", "value": 64 },
          { "label": "Meteor", "value": 65 },
          { "label": "Miami Ohio", "value": 66 },
          { "label": "Miami", "value": 67 },
          { "label": "Michigan", "value": 68 },
          { "label": "Michigan State", "value": 69 },
          { "label": "Middle Tennessee State", "value": 70 },
          { "label": "Minnesota", "value": 71 },
          { "label": "Missouri State", "value": 72 },
          { "label": "Mississippi State", "value": 73 },
          { "label": "Missouri", "value": 74 },
          { "label": "Mountain West", "value": 75 },
          { "label": "Navy", "value": 76 },
          { "label": "NCAA", "value": 77 },
          { "label": "North Carolina State", "value": 78 },
          { "label": "North Dakota State", "value": 79 },
          { "label": "Nebraska", "value": 80 },
          { "label": "Nevada", "value": 81 },
          { "label": "New Mexico", "value": 82 },
          { "label": "New Mexico State", "value": 83 },
          { "label": "NIU", "value": 84 },
          { "label": "North Carolina", "value": 134 },
          { "label": "North Texas", "value": 136 },
          { "label": "Northwestern", "value": 85 },
          { "label": "Notre Dame", "value": 86 },
          { "label": "Old Dominion", "value": 87 },
          { "label": "Ohio", "value": 88 },
          { "label": "Oklahoma State", "value": 89 },
          { "label": "Ole Miss", "value": 90 },
          { "label": "Oregon", "value": 91 },
          { "label": "Oregon State", "value": 92 },
          { "label": "Ohio State", "value": 93 },
          { "label": "Oklahoma", "value": 94 },
          { "label": "PAC 12", "value": 95 },
          { "label": "Penn State", "value": 96 },
          { "label": "Pitt", "value": 97 },
          { "label": "Purdue", "value": 98 },
          { "label": "Rice", "value": 99 },
          { "label": "Rutgers", "value": 100 },
          { "label": "Sacramento State", "value": 101 },
          { "label": "SHSU", "value": 102 },
          { "label": "SDSU", "value": 103 },
          { "label": "SEC", "value": 104 },
          { "label": "SJSU", "value": 105 },
          { "label": "SMU", "value": 106 },
          { "label": "South Alabama", "value": 107 },
          { "label": "South Carolina", "value": 108 },
          { "label": "South Florida", "value": 137 },
          { "label": "Southern Cal", "value": 109 },
          { "label": "Southern Miss", "value": 110 },
          { "label": "Stanford", "value": 111 },
          { "label": "Sun Belt", "value": 112 },
          { "label": "Syracuse", "value": 113 },
          { "label": "TCU", "value": 114 },
          { "label": "Temple", "value": 115 },
          { "label": "Tennessee", "value": 116 },
          { "label": "Texas", "value": 117 },
          { "label": "Texas A&M", "value": 118 },
          { "label": "Texas State", "value": 119 },
          { "label": "Texas Tech", "value": 120 },
          { "label": "Toledo", "value": 121 },
          { "label": "Troy", "value": 122 },
          { "label": "Tulane", "value": 123 },
          { "label": "Tulsa", "value": 124 },
          { "label": "UAB", "value": 125 },
          { "label": "UCLA", "value": 127 },
          { "label": "ULL", "value": 130 },
          { "label": "ULM", "value": 131 },
          { "label": "UNLV", "value": 135 },
          { "label": "Utah", "value": 138 },
          { "label": "Utah State", "value": 139 },
          { "label": "UTEP", "value": 140 },
          { "label": "UTSA", "value": 141 },
          { "label": "Virginia", "value": 142 },
          { "label": "Vanderbilt", "value": 143 },
          { "label": "Virginia Tech", "value": 144 },
          { "label": "Wake Forest", "value": 145 },
          { "label": "Washington", "value": 146 },
          { "label": "Washington State", "value": 147 },
          { "label": "Western Michigan", "value": 148 },
          { "label": "Wisconsin", "value": 149 },
          { "label": "Western Kentucky", "value": 150 },
          { "label": "West Virginia", "value": 151 },
          { "label": "Wyoming", "value": 152 }
        ]
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Bluetooth"
      },
      {
        "type": "select",
        "messageKey": "DisconnectVibration",
        "defaultValue": 3,
        "label": "Disconnect Vibration",
      		"options": [
      		  {
      		    "label": "Off",
      			"value": 0
      		  },
      		  {
      		    "label": "Short",
      			"value": 1
      		  },
      		  {
      		    "label": "Long",
      			"value": 2
      		  },
      		  {
      		    "label": "Double",
      			"value": 3
      		  }
      		]
      },
      {
        "type": "select",
        "messageKey": "ReconnectVibration",
        "defaultValue": 1,
        "label": "Reconnect Vibration",
      		"options": [
      		  {
      		    "label": "Off",
      			"value": 0
      		  },
      		  {
      		    "label": "Short",
      			"value": 1
      		  },
      		  {
      		    "label": "Long",
      			"value": 2
      		  },
      		  {
      		    "label": "Double",
      			"value": 3
      		  }
      		]
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Battery"
      },
  	  {
  	    "type": "slider",
  	    "messageKey": "LowBatteryPercent",
  	    "defaultValue": 30,
  	    "label": "Low Battery Percentage",
  	    "description": "Set indicator for low battery. Set 0 to turn off",
  	    "min": 0,
  	    "max": 100,
  	    "step": 1
  	  },
      {
        "type": "select",
        "messageKey": "LowBatteryVibration",
        "defaultValue": 1,
        "label": "Low Battery Vibration",
    		"options": [
    		  {
    		    "label": "Off",
    			  "value": 0
    		  },
    		  {
    		    "label": "Short",
    			  "value": 1
    		  },
    		  {
    		    "label": "Long",
    			  "value": 2
    		  },
    		  {
    		    "label": "Double",
    			  "value": 3
    		  }
    		]
      },
  	  {
  	    "type": "slider",
  	    "messageKey": "EmptyBatteryPercent",
  	    "defaultValue": 10,
  	    "label": "Very Low Battery Percentage",
  	    "description": "Set indicator for very low battery. Set 0 to turn off",
  	    "min": 0,
  	    "max": 100,
  	    "step": 1
  	  },
      {
        "type": "select",
        "messageKey": "EmptyBatteryVibration",
        "defaultValue": 2,
        "label": "Very Low Battery",
    		"options": [
    		  {
    		    "label": "Off",
    			"value": 0
    		  },
    		  {
    		    "label": "Short",
    			"value": 1
    		  },
    		  {
    		    "label": "Long",
    			"value": 2
    		  },
    		  {
    		    "label": "Double",
    			"value": 3
    		  }
    		]
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Information Update",
        "description": "Control how often to update the watchface and impact battery life."
      },
      {
        "type": "select",
        "messageKey": "watchUpdate",
        "defaultValue": 1,
        "label": "Update Frequency",
        "description": "How often should the watch update?",
    		"options": [
    		  { "label": "Every minute", "value": 1 },
    		  { "label": "Every 5 minutes", "value": 5 },
    		  { "label": "Every 10 minutes", "value": 10 },
    		  { "label": "Every 15 minutes", "value": 15 },
    		  { "label": "Every 30 minutes", "value": 30 },
    		  { "label": "Every 60 minutes", "value": 60 }
    		]
      },
      {
        "type": "toggle",
        "messageKey": "quietTimeBool",
        "label": "Enable Quiet Time",
        "description": "Used to pause options during off hours.",
        "defaultValue": false
      },
      {
        "type": "input",
        "messageKey": "quietTimeStart",
        "label": "Quiet Time Start",
        "defaultValue": "",
        "attributes": {
          "type": "time"
        }
      },
      {
        "type": "input",
        "messageKey": "quietTimeEnd",
        "label": "Quiet Time End",
        "defaultValue": "",
        "attributes": {
          "type": "time"
        }
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Animations"
      },
      {
        "type": "select",
        "messageKey": "animationSensitivity",
        "defaultValue": 1200,
        "label": "Animation Sensitivity",
    		"options": [
    		  {
    		    "label": "No Animations",
    			  "value": 0
    		  },
    		  {
    		    "label": "Low Sensitivity",
    			  "value": 2400
    		  },
    		  {
    		    "label": "Standard Sensitivity",
    			  "value": 1200
    		  },
    		  {
    		    "label": "High Sensitivity",
    			  "value": 600
    		  }
    		]
      },
      {
        "type": "toggle",
        "messageKey": "animationDelay",
        "label": "Delay Animations by 1s",
        "defaultValue": false,
        "description": "Best for viewing time in dark with backlight wake on motion"
      },
      {
        "type": "select",
        "messageKey": "animationsBatt",
        "defaultValue": 0,
        "label": "Disable Animations on Battery",
        "description": "Choose when to stop animations based on a battery percentage",
      		"options": [
      		  {
      		    "label": "Always On",
      			  "value": 0
      		  },
      		  {
      		    "label": "Low Battery",
      			  "value": 1
      		  },
      		  {
      		    "label": "Very Low Battery",
      			  "value": 2
      		  },
      		  {
      		    "label": "Custom Battery Stop",
      			  "value": 3
      		  }
      		]
      },
  	  {
  	    "type": "slider",
  	    "messageKey": "animationsCustom",
  	    "defaultValue": 30,
  	    "label": "Custom Battery Stop",
  	    "description": "Set flag for animations to stop on battery level",
  	    "min": 0,
  	    "max": 100,
  	    "step": 1
  	  }
    ]
  },

  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Countdown"
      },
      {
        "type": "toggle",
        "label": "Countdown to Gameday",
        "messageKey": "countdownBool",
        "defaultValue": false,
      },
      {
        "type": "select",
        "defaultValue": 0,
        "label": "Gametime Countdown",
        "messageKey": "countdownTime",
        "description": "What time to count down to",
      		"options": [
      		  {
      		    "label": "Saturday at Noon EST",
      			  "value": 0
      		  },
      		  {
      		    "label": "Custom",
      			  "value": 1
      		  },
      		  {
      		    "label": "Schedule - API ONLY",
      			  "value": 2
      		  }
      		]
      },
      {
        "type": "input",
        "messageKey": "countdownCustomDate",
        "label": "Custom Countdown Date",
        "defaultValue": "",
        "attributes": {
          "type": "date"
        }
      },
      {
        "type": "input",
        "messageKey": "countdownCustomTime",
        "label": "Custom Countdown Time",
        "defaultValue": "",
        "attributes": {
          "type": "time"
        }
      },
      {
        "type": "select",
        "defaultValue": 1,
        "label": "Gametime Display",
        "messageKey": "countdownDisplay",
        "description": "When to display the countdown",
      		"options": [
      		  {
      		    "label": "No Time - Only Countdown",
      			  "value": 0
      		  },
      		  {
      		    "label": "On Animation",
      			  "value": 1
      		  },
      		  {
      		    "label": "On Main Screen - Animation shows time",
      			  "value": 2
      		  }
      		]
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Health",
        "capabilities": ["HEALTH"]
      },
      {
        "type": "toggle",
        "label": "Pause Watchface Health Display During Quiet Time",
        "messageKey": "healthQuiet",
        "defaultValue": false,
        "capabilities": ["HEALTH"]
      },
      {
        "type": "toggle",
        "messageKey": "hrBool",
        "label": "Show Heart Rate",
  	    "description": "It will not show even if toggled if no heart rate is detected",
        "defaultValue": false,
        "capabilities": ["HEALTH"]
      },
      {
        "type": "toggle",
        "messageKey": "stepsBool",
        "label": "Show Steps",
        "defaultValue": false,
        "capabilities": ["HEALTH"]
      },
      {
        "type": "toggle",
        "messageKey": "stepsGoalBool",
        "label": "Show Goal Progress",
        "defaultValue": false,
        "capabilities": ["HEALTH"]
      },
  	  {
  	    "type": "slider",
  	    "messageKey": "stepsGoal",
  	    "defaultValue": 10000,
  	    "label": "Daily Step Goal",
  	    "min": 0,
  	    "max": 50000,
  	    "step": 1,
        "capabilities": ["HEALTH"]
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "API"
      },
      {
        "type": "toggle",
        "messageKey": "api",
        "label": "Auto Fetch Information",
        "defaultValue": false,
        "description": "This will make a call once per day"
      },
      {
        "type": "input",
        "label": "API Key",
        "messageKey": "api_key",
        "defaultValue": "",
  	    "description": "Get a free API key from <a href='https://collegefootballdata.com/key?source=homepage&placement=hero_supporting'>CFBD API</a>"
      },
      {
        "type": "toggle",
        "messageKey": "api_quiet",
        "label": "Pause Updates During Quite Time",
        "defaultValue": false,
        "description": "This means score updates will not occur during your quiet time"
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "API Score",
        "id": "api_score"
      },
      {
        "type": "toggle",
        "label": "Display Score During Game",
        "description": "Score will always be of Favorite Team",
        "defaultValue": false,
        "messageKey": "scoreDisplayBool"
      },
      {
        "type": "slider",
        "label": "Score Updated Frequency (in minutes)",
        "messageKey": "scoreUpdate",
        "description": "Be careful, if you constantly switch teams and have a low update frequency there is a chance to hit your API limit. Do the math.",
        "defaultValue": 5,
  	    "min": 1,
  	    "max": 60,
  	    "step": 1
      },
      {
        "type": "select",
        "defaultValue": 1,
        "label": "Score Display",
        "description": "When to display the score",
        "messageKey": "scoreLocation",
      		"options": [
      		  {
      		    "label": "No Time - Only Score",
      			  "value": 0
      		  },
      		  {
      		    "label": "On Animation",
      			  "value": 1
      		  },
      		  {
      		    "label": "On Main Screen - Animation shows time",
      			  "value": 2
      		  }
      		]
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "API Extras",
        "id": "api_extras"
      },
      {
        "type": "text",
        "id": "superlatives",
        "defaultValue": "Add small markings for season superlatives"
      },
      {
        "type": "toggle",
        "label": "Display Ranking",
        "defaultValue": false,
        "messageKey": "rankingBool"
      },
      {
        "type": "toggle",
        "label": "Winning Season Marking",
        "defaultValue": false,
        "messageKey": "winBool"
      },
      {
        "type": "toggle",
        "label": "Conference Champion Marking",
        "defaultValue": false,
        "messageKey": "confBool"
      },
      {
        "type": "toggle",
        "label": "Bowl Win Marking",
        "defaultValue": false,
        "messageKey": "bowlBool"
      },
      {
        "type": "toggle",
        "label": "Champion Marking",
        "defaultValue": false,
        "messageKey": "champBool"
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Weather"
      },
      {
        "type": "toggle",
        "label": "Weather",
        "messageKey": "weatherBool",
        "defaultValue": false,
      },
      {
        "type": "toggle",
        "label": "Pause Updates During Quiet Time",
        "messageKey": "weatherQuiet",
        "defaultValue": false,
      },
      {
	      "type": "radiogroup",
	      "label": "Units",
        "messageKey": "weatherUnits",
		    "defaultValue": "0",
	      "options": [
	  	    { 
		        "label": "Fahrenheit", 
		        "value": "0"
		      },
		      { 
		        "label": "Celsius", 
		        "value": "1"
		      }
		    ]
	    }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Donation"
      },
      {
        "type": "toggle",
        "id": "donate",
        "label": "Donation",
  	    "description": "Consider donating to help support me!",
        "defaultValue": false
      },
      {
        "type": "text",
        "id": "donation_block",
        "defaultValue": "<div style='text-align: center; margin-top: 20px;'>\n  <p>If you like this watchface, consider supporting my work!</p>\n  <a href='https://www.paypal.me/kennylinderman1/5' target='_blank'>\n    <img src='https://cdn.buymeacoffee.com/buttons/v2/default-blue.png' alt='Buy Me A Coffee' style='height: 50px !important; width: 217px !important;' />\n  </a>\n</div>"
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];
