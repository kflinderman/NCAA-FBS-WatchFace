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
		    "defaultValue": 0,
	      "options": [
	  	    { 
		        "label": "Favorite Team", 
		        "value": 0
		      },
		      { 
		        "label": "Team You Want to Beat", 
		        "value": 1
		      }
		    ]
	    },
	    {
		    "type": "select",
		    "messageKey": "FavoriteTeam",
		    "defaultValue": 0,
		    "label": "Favorite Team",
		    "options": [
		      { 
		        "label": "South Carolina", 
		        "value": 0
		      }
		    ]
	    },
	    {
		    "type": "select",
		    "messageKey": "BeatTeam",
		    "defaultValue": 1,
		    "label": "Team You Want to Beat",
		    "options": [
		      { 
		        "label": "Clemson", 
		        "value": 1
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
  		  "defaultValue": 0,
  	    "options": [
  	  	  { 
  		      "label": "5.00", 
  		      "value": 500
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
