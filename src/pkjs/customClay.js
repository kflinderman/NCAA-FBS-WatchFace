module.exports = function (minified) {
  var clayConfig = this;

  /**********************/
  /* Condition Helpers  */
  /**********************/

  var isTrue = function (v) { return v === true; };
  var isFalse = function (v) { return v === false; };
  var eq = function (target) {
    return function (v) { return parseInt(v, 10) === target; };
  };
  var neq = function (target) {
    return function (v) { return parseInt(v, 10) !== target; };
  };

  /**********************/
  /* Dependency Tree   */
  /**********************/

  // Configuration tree defining conditional visibility and parent-child field hierarchies
  var tree = [
    { key: 'DisplayTeam' },
    { key: 'bagBool' },
    { key: 'FavoriteTeam' },
    {
      key: 'hardcodeRivalBool',
      condition: eq(0), // display below on 0
      children: [
        { key: 'BeatTeam' }
      ]
    },
    {
      key: 'hardcodeRivalBool',
      condition: eq(2), // display below on 2
      children: [
        {
          key: 'opponentSelect',
          condition: eq(2), // display below on 2
          children: [
            { key: 'customOpponent' }
          ]
        }
      ]
    },
    { key: 'DisconnectVibration' },
    { key: 'ReconnectVibration' },
    {
      key: 'LowBatteryPercent',
      condition: neq(0), // hide below on 0
      children: [
        { key: 'LowBatteryVibration' }
      ]
    },
    {
      key: 'EmptyBatteryPercent',
      condition: neq(0), // hide below on 0
      children: [
        { key: 'EmptyBatteryVibration' }
      ]
    },
    {
      key: 'quietTimeBool',
      condition: isTrue, // display below on true
      children: [
        { key: 'quietTimeStart' },
        { key: 'quietTimeEnd' }
      ]
    },
    {
      key: 'animationSensitivity',
      condition: neq(0), // hide below on 0
      children: [
        { key: 'animationDelay' },
        {
          key: 'animationsBatt',
          condition: eq(3), // display below on 3
          children: [
            { key: 'animationsCustom' }
          ]
        }
      ]
    },
    {
      key: 'countdownBool',
      condition: isTrue, // display below on true
      children: [
        {
          key: 'countdownTime',
          condition: eq(1), // display below on 1
          children: [
            { key: 'countdownCustomDate' },
            { key: 'countdownCustomTime' }
          ]
        },
        { key: 'countdownDisplay' }
      ]
    },
    { key: 'healthQuiet', externalKey: 'quietTimeBool' }, // only when quietTimeBool is also true
    { key: 'hrBool' },
    { key: 'stepsBool' },
    {
      key: 'stepsGoalBool',
      condition: isTrue, // display below on true
      children: [
        { key: 'stepsGoal' }
      ]
    },
    {
      key: 'api',
      condition: isTrue, // display below on true
      children: [
        { key: 'api_key' },
        { key: 'api_quiet', externalKey: 'quietTimeBool' }, // only when quietTimeBool is also true
        { key: 'api_score' },
        {
          key: 'scoreDisplayBool',
          condition: isTrue, // display below on true
          children: [
            { key: 'scoreUpdate' },
            { key: 'scoreLocation' }
          ]
        },
        { key: 'api_extras' },
        { key: 'superlatives'},
        { key: 'rankingBool' },
        { key: 'winBool' },
        { key: 'confBool' },
        { key: 'bowlBool' },
        { key: 'champBool' }
      ]
    },
    {
      key: 'weatherBool',
      condition: isTrue, // display below on true
      children: [
        { key: 'weatherQuiet', externalKey: 'quietTimeBool' }, // only when quietTimeBool is also true
        { key: 'weatherUnits' },
      ]
    },
    {
      key: 'donate',
      condition: isTrue, // display below on true
      children: [
        { key: 'donation_block' }
      ]
    }
  ];

  /**********************/
  /* Visibility Logic   */
  /**********************/

  // Helper function to resolve a Clay configuration item by message key or element ID
  function getItem(key) {
    var item = clayConfig.getItemByMessageKey(key);
    if (!item) {
      item = clayConfig.getItemById(key);
    }
    return item;
  }

  // Recursively apply visibility rules to a node and all of its nested children
  function applyNode(node, parentVisible) {
    var item = getItem(node.key);
    var ownVisible = parentVisible;

    // Check external dependency rule if specified
    if (ownVisible && node.externalKey) {
      var extItem = getItem(node.externalKey);
      if (extItem) {
        ownVisible = ownVisible && !!extItem.get();
      } else {
        console.log("Error: Could not find external control '" + node.externalKey + "' for '" + node.key + "'!");
      }
    }

    // Toggle field visibility in the DOM
    if (item) {
      ownVisible ? item.show() : item.hide();
    } else {
      console.log("Error: Could not find item '" + node.key + "' by Key or ID!");
    }

    // Process dependent child nodes
    if (node.children && node.children.length) {
      var childrenVisible = ownVisible;
      if (ownVisible && node.condition) {
        childrenVisible = item ? node.condition(item.get()) : false;
      }
      node.children.forEach(function (child) {
        applyNode(child, childrenVisible);
      });
    }
  }

  // Re-evaluate visibility states across the entire dependency tree
  function recomputeAll() {
    tree.forEach(function (node) {
      applyNode(node, true);
    });
  }

  // Recursively register change listeners on configurable items to trigger UI updates
  function attachListeners(nodes) {
    nodes.forEach(function (node) {
      var item = getItem(node.key);
      if (item) {
        item.on('change', recomputeAll);
      }
      if (node.children && node.children.length) {
        attachListeners(node.children);
      }
    });
  }

  /**********************/
  /* Event Handlers     */
  /**********************/

  // Hook into Clay build lifecycle after DOM elements are fully initialized
  clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function () {
    attachListeners(tree);
    recomputeAll();

    var watchUpdateItem = getItem('watchUpdate');
    var scoreUpdateItem = getItem('scoreUpdate');

    // Dynamic constraint sync between watch update rate and score update rate
    if (watchUpdateItem && scoreUpdateItem) {
      function syncScoreMin() {
        // Get the current integer value of watchUpdate
        var newMin = parseInt(watchUpdateItem.get(), 10);

        // 1. Update the internal Clay config
        scoreUpdateItem.config.min = newMin;

        // 2. Update the actual HTML attribute on the range input
        if (scoreUpdateItem.$manipulator) {
          scoreUpdateItem.$manipulator.set('@min', newMin);
        }

        // 3. Ensure the current slider value doesn't violate the new minimum
        if (parseInt(scoreUpdateItem.get(), 10) < newMin) {
          scoreUpdateItem.set(newMin); // Automatically bumps slider up
        }
      }

      // Trigger whenever the 'watchUpdate' select menu changes
      watchUpdateItem.on('change', syncScoreMin);

      // Run once on load to ensure initial state is correct
      syncScoreMin();
    }
  });
};