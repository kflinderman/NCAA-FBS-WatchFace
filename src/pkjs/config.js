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
		        "value": "1"
		      }
		    ]
	    },
	    {
		    "type": "select",
		    "messageKey": "FavoriteTeam",
		    "defaultValue": 108,
		    "label": "Favorite Team",
		    "options": [
		      { 
		        "label": "ACC", 
		        "value": 0
		      },
		      { 
		        "label": "Air Force", 
		        "value": 1
		      },
		      { 
		        "label": "Akron", 
		        "value": 2
		      },
		      { 
		        "label": "Alabama", 
		        "value": 3
		      },
		      { 
		        "label": "American", 
		        "value": 4
		      },
		      { 
		        "label": "App State", 
		        "value": 5
		      },
		      { 
		        "label": "Arizona", 
		        "value": 6
		      },
		      { 
		        "label": "Arizona State", 
		        "value": 7
		      },
		      { 
		        "label": "Arkansas", 
		        "value": 8
		      },
		      { 
		        "label": "Arkansas State", 
		        "value": 9
		      },
		      { 
		        "label": "Army", 
		        "value": 10
		      },
		      { 
		        "label": "Auburn", 
		        "value": 11
		      },
		      { 
		        "label": "B1G", 
		        "value": 12
		      },
		      { 
		        "label": "Ball State", 
		        "value": 13
		      },
		      { 
		        "label": "Baylor", 
		        "value": 14
		      },
		      { 
		        "label": "BIG 12", 
		        "value": 15
		      },
		      { 
		        "label": "Boise State", 
		        "value": 16
		      },
		      { 
		        "label": "Boston College", 
		        "value": 17
		      },
		      { 
		        "label": "Bowling Green", 
		        "value": 18
		      },
		      { 
		        "label": "Buffalo", 
		        "value": 19
		      },
		      { 
		        "label": "BYU", 
		        "value": 20
		      },
		      { 
		        "label": "California", 
		        "value": 21
		      },
		      { 
		        "label": "Central Florida", 
		        "value": 126
		      },
		      { 
		        "label": "Central Michigan", 
		        "value": 22
		      },
		      { 
		        "label": "CFB", 
		        "value": 23
		      },
		      { 
		        "label": "Chaos", 
		        "value": 24
		      },
		      { 
		        "label": "Charlotte", 
		        "value": 133
		      },
		      { 
		        "label": "Cincinnati", 
		        "value": 25
		      },
		      { 
		        "label": "Clemson", 
		        "value": 26
		      },
		      { 
		        "label": "Coastal Carolina", 
		        "value": 27
		      },
		      { 
		        "label": "Colorado", 
		        "value": 28
		      },
		      { 
		        "label": "Colorado State", 
		        "value": 29
		      },
		      { 
		        "label": "Connecticut", 
		        "value": 128
		      },
		      { 
		        "label": "CUSA", 
		        "value": 30
		      },
		      { 
		        "label": "Deleware", 
		        "value": 31
		      },
		      { 
		        "label": "Duke", 
		        "value": 32
		      },
		      { 
		        "label": "Eastern Michigan", 
		        "value": 33
		      },
		      { 
		        "label": "East Carolina", 
		        "value": 34
		      },
		      { 
		        "label": "FAU", 
		        "value": 35
		      },
		      { 
		        "label": "Florida International", 
		        "value": 36
		      },
		      { 
		        "label": "Florida", 
		        "value": 37
		      },
		      { 
		        "label": "Fresno State", 
		        "value": 38
		      },
		      { 
		        "label": "Florida State", 
		        "value": 39
		      },
		      { 
		        "label": "Georgia", 
		        "value": 129
		      },
		      { 
		        "label": "Georgia Southern", 
		        "value": 40
		      },
		      { 
		        "label": "Georgia State", 
		        "value": 41
		      },
		      { 
		        "label": "Georgia Tech", 
		        "value": 42
		      },
		      { 
		        "label": "Hawaii", 
		        "value": 43
		      },
		      { 
		        "label": "Houston", 
		        "value": 44
		      },
		      { 
		        "label": "Illinois", 
		        "value": 45
		      },
		      { 
		        "label": "Independent", 
		        "value": 46
		      },
		      { 
		        "label": "Indiana", 
		        "value": 47
		      },
		      { 
		        "label": "Iowa", 
		        "value": 48
		      },
		      { 
		        "label": "Iowa State", 
		        "value": 49
		      },
		      { 
		        "label": "Jacksonville State", 
		        "value": 50
		      },
		      { 
		        "label": "James Maddison", 
		        "value": 51
		      },
		      { 
		        "label": "Kansas", 
		        "value": 52
		      },
		      { 
		        "label": "Kennesaw State", 
		        "value": 53
		      },
		      { 
		        "label": "Kent State", 
		        "value": 54
		      },
		      { 
		        "label": "Kentucky", 
		        "value": 55
		      },
		      { 
		        "label": "Kansas State", 
		        "value": 56
		      },
		      { 
		        "label": "LA Tech", 
		        "value": 57
		      },
		      { 
		        "label": "Liberty", 
		        "value": 58
		      },
		      { 
		        "label": "Louisville", 
		        "value": 59
		      },
		      { 
		        "label": "LSU", 
		        "value": 60
		      },
		      { 
		        "label": "MAC", 
		        "value": 61
		      },
		      { 
		        "label": "Marshall", 
		        "value": 62
		      },
		      { 
		        "label": "Maryland", 
		        "value": 63
		      },
		      { 
		        "label": "Massachusetts", 
		        "value": 132
		      },
		      { 
		        "label": "Memphis", 
		        "value": 64
		      },
		      { 
		        "label": "Meteor", 
		        "value": 65
		      },
		      { 
		        "label": "Miami Ohio", 
		        "value": 66
		      },
		      { 
		        "label": "Miami", 
		        "value": 67
		      },
		      { 
		        "label": "Michigan", 
		        "value": 68
		      },
		      { 
		        "label": "Michigan State", 
		        "value": 69
		      },
		      { 
		        "label": "Middle Tennessee State", 
		        "value": 70
		      },
		      { 
		        "label": "Minnesota", 
		        "value": 71
		      },
		      { 
		        "label": "Missouri State", 
		        "value": 72
		      },
		      { 
		        "label": "Mississippi State", 
		        "value": 73
		      },
		      { 
		        "label": "Missouri", 
		        "value": 74
		      },
		      { 
		        "label": "Mountain West", 
		        "value": 75
		      },
		      { 
		        "label": "Navy", 
		        "value": 76
		      },
		      { 
		        "label": "NCAA", 
		        "value": 77
		      },
		      { 
		        "label": "North Carolina State", 
		        "value": 78
		      },
		      { 
		        "label": "North Dakota State", 
		        "value": 79
		      },
		      { 
		        "label": "Nebraska", 
		        "value": 80
		      },
		      { 
		        "label": "Nevada", 
		        "value": 81
		      },
		      { 
		        "label": "New Mexico", 
		        "value": 82
		      },
		      { 
		        "label": "New Mexico State", 
		        "value": 83
		      },
		      { 
		        "label": "NIU", 
		        "value": 84
		      },
		      { 
		        "label": "North Carolina", 
		        "value": 134
		      },
		      { 
		        "label": "North Texas", 
		        "value": 136
		      },
		      { 
		        "label": "Northwestern", 
		        "value": 85
		      },
		      { 
		        "label": "Notre Dame", 
		        "value": 86
		      },
		      { 
		        "label": "Old Dominion", 
		        "value": 87
		      },
		      { 
		        "label": "Ohio", 
		        "value": 88
		      },
		      { 
		        "label": "Oklahoma State", 
		        "value": 89
		      },
		      { 
		        "label": "Ole Miss", 
		        "value": 90
		      },
		      { 
		        "label": "Oregon", 
		        "value": 91
		      },
		      { 
		        "label": "Oregon State", 
		        "value": 92
		      },
		      { 
		        "label": "Ohio State", 
		        "value": 93
		      },
		      { 
		        "label": "Oklahoma", 
		        "value": 94
		      },
		      { 
		        "label": "PAC 12", 
		        "value": 95
		      },
		      { 
		        "label": "Penn State", 
		        "value": 96
		      },
		      { 
		        "label": "Pitt", 
		        "value": 97
		      },
		      { 
		        "label": "Purdue", 
		        "value": 98
		      },
		      { 
		        "label": "Rice", 
		        "value": 99
		      },
		      { 
		        "label": "Rutgers", 
		        "value": 100
		      },
		      { 
		        "label": "Sacramento State", 
		        "value": 101
		      },
		      { 
		        "label": "SHSU", 
		        "value": 102
		      },
		      { 
		        "label": "SDSU", 
		        "value": 103
		      },
		      { 
		        "label": "SEC", 
		        "value": 104
		      },
		      { 
		        "label": "SJSU", 
		        "value": 105
		      },
		      { 
		        "label": "SMU", 
		        "value": 106
		      },
		      { 
		        "label": "South Alabama", 
		        "value": 107
		      },
		      { 
		        "label": "South Carolina", 
		        "value": 108
		      },
		      { 
		        "label": "South Florida", 
		        "value": 137
		      },
		      { 
		        "label": "Southern Cal", 
		        "value": 109
		      },
		      { 
		        "label": "Southern Miss", 
		        "value": 110
		      },
		      { 
		        "label": "Stanford", 
		        "value": 111
		      },
		      { 
		        "label": "Sun Belt", 
		        "value": 112
		      },
		      { 
		        "label": "Syracuse", 
		        "value": 113
		      },
		      { 
		        "label": "TCU", 
		        "value": 114
		      },
		      { 
		        "label": "Temple", 
		        "value": 115
		      },
		      { 
		        "label": "Tennessee", 
		        "value": 116
		      },
		      { 
		        "label": "Texas", 
		        "value": 117
		      },
		      { 
		        "label": "Texas A&M", 
		        "value": 118
		      },
		      { 
		        "label": "Texas State", 
		        "value": 119
		      },
		      { 
		        "label": "Texas Tech", 
		        "value": 120
		      },
		      { 
		        "label": "Toledo", 
		        "value": 121
		      },
		      { 
		        "label": "Troy", 
		        "value": 122
		      },
		      { 
		        "label": "Tulane", 
		        "value": 123
		      },
		      { 
		        "label": "Tulsa", 
		        "value": 124
		      },
		      { 
		        "label": "UAB", 
		        "value": 125
		      },
		      { 
		        "label": "UCLA", 
		        "value": 127
		      },
		      { 
		        "label": "ULL", 
		        "value": 130
		      },
		      { 
		        "label": "ULM", 
		        "value": 131
		      },
		      { 
		        "label": "UNLV", 
		        "value": 135
		      },
		      { 
		        "label": "Utah", 
		        "value": 138
		      },
		      { 
		        "label": "Utah State", 
		        "value": 139
		      },
		      { 
		        "label": "UTEP", 
		        "value": 140
		      },
		      { 
		        "label": "UTSA", 
		        "value": 141
		      },
		      { 
		        "label": "Virginia", 
		        "value": 142
		      },
		      { 
		        "label": "Vanderbilt", 
		        "value": 143
		      },
		      { 
		        "label": "Virginia Tech", 
		        "value": 144
		      },
		      { 
		        "label": "Wake Forest", 
		        "value": 145
		      },
		      { 
		        "label": "Washington", 
		        "value": 146
		      },
		      { 
		        "label": "Washington State", 
		        "value": 147
		      },
		      { 
		        "label": "Western Michigan", 
		        "value": 148
		      },
		      { 
		        "label": "Wisconsin", 
		        "value": 149
		      },
		      { 
		        "label": "Western Kentucky", 
		        "value": 150
		      },
		      { 
		        "label": "West Virginia", 
		        "value": 151
		      },
		      { 
		        "label": "Wyoming", 
		        "value": 152
		      }
		    ]
	    },
	    {
		    "type": "select",
		    "messageKey": "BeatTeam",
		    "defaultValue": 26,
		    "label": "Team You Want to Beat",
		    "options": [
		      { 
		        "label": "ACC", 
		        "value": 0
		      },
		      { 
		        "label": "Air Force", 
		        "value": 1
		      },
		      { 
		        "label": "Akron", 
		        "value": 2
		      },
		      { 
		        "label": "Alabama", 
		        "value": 3
		      },
		      { 
		        "label": "American", 
		        "value": 4
		      },
		      { 
		        "label": "App State", 
		        "value": 5
		      },
		      { 
		        "label": "Arizona", 
		        "value": 6
		      },
		      { 
		        "label": "Arizona State", 
		        "value": 7
		      },
		      { 
		        "label": "Arkansas", 
		        "value": 8
		      },
		      { 
		        "label": "Arkansas State", 
		        "value": 9
		      },
		      { 
		        "label": "Army", 
		        "value": 10
		      },
		      { 
		        "label": "Auburn", 
		        "value": 11
		      },
		      { 
		        "label": "B1G", 
		        "value": 12
		      },
		      { 
		        "label": "Ball State", 
		        "value": 13
		      },
		      { 
		        "label": "Baylor", 
		        "value": 14
		      },
		      { 
		        "label": "BIG 12", 
		        "value": 15
		      },
		      { 
		        "label": "Boise State", 
		        "value": 16
		      },
		      { 
		        "label": "Boston College", 
		        "value": 17
		      },
		      { 
		        "label": "Bowling Green", 
		        "value": 18
		      },
		      { 
		        "label": "Buffalo", 
		        "value": 19
		      },
		      { 
		        "label": "BYU", 
		        "value": 20
		      },
		      { 
		        "label": "California", 
		        "value": 21
		      },
		      { 
		        "label": "Central Florida", 
		        "value": 126
		      },
		      { 
		        "label": "Central Michigan", 
		        "value": 22
		      },
		      { 
		        "label": "CFB", 
		        "value": 23
		      },
		      { 
		        "label": "Chaos", 
		        "value": 24
		      },
		      { 
		        "label": "Charlotte", 
		        "value": 133
		      },
		      { 
		        "label": "Cincinnati", 
		        "value": 25
		      },
		      { 
		        "label": "Clemson", 
		        "value": 26
		      },
		      { 
		        "label": "Coastal Carolina", 
		        "value": 27
		      },
		      { 
		        "label": "Colorado", 
		        "value": 28
		      },
		      { 
		        "label": "Colorado State", 
		        "value": 29
		      },
		      { 
		        "label": "Connecticut", 
		        "value": 128
		      },
		      { 
		        "label": "CUSA", 
		        "value": 30
		      },
		      { 
		        "label": "Deleware", 
		        "value": 31
		      },
		      { 
		        "label": "Duke", 
		        "value": 32
		      },
		      { 
		        "label": "Eastern Michigan", 
		        "value": 33
		      },
		      { 
		        "label": "East Carolina", 
		        "value": 34
		      },
		      { 
		        "label": "FAU", 
		        "value": 35
		      },
		      { 
		        "label": "Florida International", 
		        "value": 36
		      },
		      { 
		        "label": "Florida", 
		        "value": 37
		      },
		      { 
		        "label": "Fresno State", 
		        "value": 38
		      },
		      { 
		        "label": "Florida State", 
		        "value": 39
		      },
		      { 
		        "label": "Georgia", 
		        "value": 129
		      },
		      { 
		        "label": "Georgia Southern", 
		        "value": 40
		      },
		      { 
		        "label": "Georgia State", 
		        "value": 41
		      },
		      { 
		        "label": "Georgia Tech", 
		        "value": 42
		      },
		      { 
		        "label": "Hawaii", 
		        "value": 43
		      },
		      { 
		        "label": "Houston", 
		        "value": 44
		      },
		      { 
		        "label": "Illinois", 
		        "value": 45
		      },
		      { 
		        "label": "Independent", 
		        "value": 46
		      },
		      { 
		        "label": "Indiana", 
		        "value": 47
		      },
		      { 
		        "label": "Iowa", 
		        "value": 48
		      },
		      { 
		        "label": "Iowa State", 
		        "value": 49
		      },
		      { 
		        "label": "Jacksonville State", 
		        "value": 50
		      },
		      { 
		        "label": "James Maddison", 
		        "value": 51
		      },
		      { 
		        "label": "Kansas", 
		        "value": 52
		      },
		      { 
		        "label": "Kennesaw State", 
		        "value": 53
		      },
		      { 
		        "label": "Kent State", 
		        "value": 54
		      },
		      { 
		        "label": "Kentucky", 
		        "value": 55
		      },
		      { 
		        "label": "Kansas State", 
		        "value": 56
		      },
		      { 
		        "label": "LA Tech", 
		        "value": 57
		      },
		      { 
		        "label": "Liberty", 
		        "value": 58
		      },
		      { 
		        "label": "Louisville", 
		        "value": 59
		      },
		      { 
		        "label": "LSU", 
		        "value": 60
		      },
		      { 
		        "label": "MAC", 
		        "value": 61
		      },
		      { 
		        "label": "Marshall", 
		        "value": 62
		      },
		      { 
		        "label": "Maryland", 
		        "value": 63
		      },
		      { 
		        "label": "Massachusetts", 
		        "value": 132
		      },
		      { 
		        "label": "Memphis", 
		        "value": 64
		      },
		      { 
		        "label": "Meteor", 
		        "value": 65
		      },
		      { 
		        "label": "Miami Ohio", 
		        "value": 66
		      },
		      { 
		        "label": "Miami", 
		        "value": 67
		      },
		      { 
		        "label": "Michigan", 
		        "value": 68
		      },
		      { 
		        "label": "Michigan State", 
		        "value": 69
		      },
		      { 
		        "label": "Middle Tennessee State", 
		        "value": 70
		      },
		      { 
		        "label": "Minnesota", 
		        "value": 71
		      },
		      { 
		        "label": "Missouri State", 
		        "value": 72
		      },
		      { 
		        "label": "Mississippi State", 
		        "value": 73
		      },
		      { 
		        "label": "Missouri", 
		        "value": 74
		      },
		      { 
		        "label": "Mountain West", 
		        "value": 75
		      },
		      { 
		        "label": "Navy", 
		        "value": 76
		      },
		      { 
		        "label": "NCAA", 
		        "value": 77
		      },
		      { 
		        "label": "North Carolina State", 
		        "value": 78
		      },
		      { 
		        "label": "North Dakota State", 
		        "value": 79
		      },
		      { 
		        "label": "Nebraska", 
		        "value": 80
		      },
		      { 
		        "label": "Nevada", 
		        "value": 81
		      },
		      { 
		        "label": "New Mexico", 
		        "value": 82
		      },
		      { 
		        "label": "New Mexico State", 
		        "value": 83
		      },
		      { 
		        "label": "NIU", 
		        "value": 84
		      },
		      { 
		        "label": "North Carolina", 
		        "value": 134
		      },
		      { 
		        "label": "North Texas", 
		        "value": 136
		      },
		      { 
		        "label": "Northwestern", 
		        "value": 85
		      },
		      { 
		        "label": "Notre Dame", 
		        "value": 86
		      },
		      { 
		        "label": "Old Dominion", 
		        "value": 87
		      },
		      { 
		        "label": "Ohio", 
		        "value": 88
		      },
		      { 
		        "label": "Oklahoma State", 
		        "value": 89
		      },
		      { 
		        "label": "Ole Miss", 
		        "value": 90
		      },
		      { 
		        "label": "Oregon", 
		        "value": 91
		      },
		      { 
		        "label": "Oregon State", 
		        "value": 92
		      },
		      { 
		        "label": "Ohio State", 
		        "value": 93
		      },
		      { 
		        "label": "Oklahoma", 
		        "value": 94
		      },
		      { 
		        "label": "PAC 12", 
		        "value": 95
		      },
		      { 
		        "label": "Penn State", 
		        "value": 96
		      },
		      { 
		        "label": "Pitt", 
		        "value": 97
		      },
		      { 
		        "label": "Purdue", 
		        "value": 98
		      },
		      { 
		        "label": "Rice", 
		        "value": 99
		      },
		      { 
		        "label": "Rutgers", 
		        "value": 100
		      },
		      { 
		        "label": "Sacramento State", 
		        "value": 101
		      },
		      { 
		        "label": "SHSU", 
		        "value": 102
		      },
		      { 
		        "label": "SDSU", 
		        "value": 103
		      },
		      { 
		        "label": "SEC", 
		        "value": 104
		      },
		      { 
		        "label": "SJSU", 
		        "value": 105
		      },
		      { 
		        "label": "SMU", 
		        "value": 106
		      },
		      { 
		        "label": "South Alabama", 
		        "value": 107
		      },
		      { 
		        "label": "South Carolina", 
		        "value": 108
		      },
		      { 
		        "label": "South Florida", 
		        "value": 137
		      },
		      { 
		        "label": "Southern Cal", 
		        "value": 109
		      },
		      { 
		        "label": "Southern Miss", 
		        "value": 110
		      },
		      { 
		        "label": "Stanford", 
		        "value": 111
		      },
		      { 
		        "label": "Sun Belt", 
		        "value": 112
		      },
		      { 
		        "label": "Syracuse", 
		        "value": 113
		      },
		      { 
		        "label": "TCU", 
		        "value": 114
		      },
		      { 
		        "label": "Temple", 
		        "value": 115
		      },
		      { 
		        "label": "Tennessee", 
		        "value": 116
		      },
		      { 
		        "label": "Texas", 
		        "value": 117
		      },
		      { 
		        "label": "Texas A&M", 
		        "value": 118
		      },
		      { 
		        "label": "Texas State", 
		        "value": 119
		      },
		      { 
		        "label": "Texas Tech", 
		        "value": 120
		      },
		      { 
		        "label": "Toledo", 
		        "value": 121
		      },
		      { 
		        "label": "Troy", 
		        "value": 122
		      },
		      { 
		        "label": "Tulane", 
		        "value": 123
		      },
		      { 
		        "label": "Tulsa", 
		        "value": 124
		      },
		      { 
		        "label": "UAB", 
		        "value": 125
		      },
		      { 
		        "label": "UCLA", 
		        "value": 127
		      },
		      { 
		        "label": "ULL", 
		        "value": 130
		      },
		      { 
		        "label": "ULM", 
		        "value": 131
		      },
		      { 
		        "label": "UNLV", 
		        "value": 135
		      },
		      { 
		        "label": "Utah", 
		        "value": 138
		      },
		      { 
		        "label": "Utah State", 
		        "value": 139
		      },
		      { 
		        "label": "UTEP", 
		        "value": 140
		      },
		      { 
		        "label": "UTSA", 
		        "value": 141
		      },
		      { 
		        "label": "Virginia", 
		        "value": 142
		      },
		      { 
		        "label": "Vanderbilt", 
		        "value": 143
		      },
		      { 
		        "label": "Virginia Tech", 
		        "value": 144
		      },
		      { 
		        "label": "Wake Forest", 
		        "value": 145
		      },
		      { 
		        "label": "Washington", 
		        "value": 146
		      },
		      { 
		        "label": "Washington State", 
		        "value": 147
		      },
		      { 
		        "label": "Western Michigan", 
		        "value": 148
		      },
		      { 
		        "label": "Wisconsin", 
		        "value": 149
		      },
		      { 
		        "label": "Western Kentucky", 
		        "value": 150
		      },
		      { 
		        "label": "West Virginia", 
		        "value": 151
		      },
		      { 
		        "label": "Wyoming", 
		        "value": 152
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
        "defaultValue": "Developer"
      },
  	  {
  	    "type": "radiogroup",
  	    "messageKey": "Version",
  	    "label": "Version",
  		  "defaultValue": "500",
  	    "options": [
  	  	  { 
  		      "label": "5.00", 
  		      "value": "500"
  		    }
		    ]
	    }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];
