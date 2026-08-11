module.exports = function (minified) {
    var clayConfig = this;

    // ------------------------------------------------------------------
    // Visibility tree
    // ------------------------------------------------------------------
    // Each node represents one Clay item.
    //   key:             messageKey (or id, for text/donation_block items)
    //   condition:       fn(rawValue) -> bool. If present, controls whether
    //                    this node's CHILDREN are shown (this node must
    //                    itself be visible for the condition to matter).
    //   externalKey:     messageKey of ANOTHER item elsewhere in the tree
    //                    whose truthiness gates this node's OWN visibility,
    //                    in addition to its parent's visibility.
    //   children:        nested nodes, hidden automatically if this node
    //                    is hidden (cascading), and further filtered by
    //                    `condition` if present.
    //
    // A node is visible only if every ancestor is visible AND every
    // ancestor's condition (if any) evaluates true against that ancestor's
    // current value. That's what makes "higher hides everything below it"
    // work automatically, no matter how deep the nesting.
    // ------------------------------------------------------------------

    var isTrue = function (v) { return v === true; };
    var isFalse = function (v) { return v === false; };
    var eq = function (target) {
        return function (v) { return parseInt(v, 10) === target; };
    };
    var neq = function (target) {
        return function (v) { return parseInt(v, 10) !== target; };
    };

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

    // ------------------------------------------------------------------
    // Engine
    // ------------------------------------------------------------------

    function getItem(key) {
        var item = clayConfig.getItemByMessageKey(key);
        if (!item) {
            item = clayConfig.getItemById(key);
        }
        return item;
    }

    // Recursively applies visibility down the tree.
    // parentVisible = whether this node's parent chain allows it to show.
    function applyNode(node, parentVisible) {
        var item = getItem(node.key);
        var ownVisible = parentVisible;

        if (ownVisible && node.externalKey) {
            var extItem = getItem(node.externalKey);
            if (extItem) {
                ownVisible = ownVisible && !!extItem.get();
            } else {
                console.log("Error: Could not find external control '" + node.externalKey + "' for '" + node.key + "'!");
            }
        }

        if (item) {
            ownVisible ? item.show() : item.hide();
        } else {
            console.log("Error: Could not find item '" + node.key + "' by Key or ID!");
        }

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

    function recomputeAll() {
        tree.forEach(function (node) {
            applyNode(node, true);
        });
    }

    // Attach a 'change' listener to every node in the tree (any node's
    // value could gate its own children, or be an externalKey target for
    // something elsewhere), and just recompute the whole tree each time.
    // This guarantees correct cascading no matter how deep or tangled
    // the dependencies get.
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

clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function () {
        attachListeners(tree);
        recomputeAll();

        // --- NEW CODE: Sync scoreUpdate min to watchUpdate value ---
        
        var watchUpdateItem = getItem('watchUpdate');
        var scoreUpdateItem = getItem('scoreUpdate');

        if (watchUpdateItem && scoreUpdateItem) {
            function syncScoreMin() {
                // Get the current integer value of watchUpdate
                var newMin = parseInt(watchUpdateItem.get(), 10);

                // 1. Update the internal Clay config
                scoreUpdateItem.config.min = newMin;

                // 2. Update the actual HTML attribute on the range input
                // (Clay components expose their input elements via $manipulator)
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