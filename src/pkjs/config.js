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
		    "defaultValue": 1,
		    "label": "Favorite Team",
		    "options": [
		      { "label": "Clemson", "value": 0 },
		      { "label": "South Carolina", "value": 1 }
		    ]
	    },
      {
        "type": "toggle",
        "messageKey": "hardcodeRivalBool",
        "label": "Beat your Rival?",
        "defaultValue": true
      },
	    {
		    "type": "select",
		    "messageKey": "BeatTeam",
		    "defaultValue": 0,
		    "label": "Beat Team",
		    "options": [
		      { "label": "Clemson", "value": 0 },
		      { "label": "South Carolina", "value": 1 }
		    ]
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
        "messageKey": "quietTimeBool",
        "label": "Enable Quiet Time",
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
        "defaultValue": "Health",
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
        "defaultValue": "Donation"
      },
      {
        "type": "toggle",
        "messageKey": "donate",
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