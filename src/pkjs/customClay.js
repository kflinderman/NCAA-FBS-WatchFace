module.exports = function (minified) {
    var clayConfig = this;
    
    // Generic toggle handler factory
    function createToggleHandler(config) {
        return function() {
            var currentValue = config.isBoolean ? !!this.get() : parseInt(this.get(), 10);
            var shouldShow = config.showWhen(currentValue);
            
            config.items.forEach(function(itemKey) {
                // 1. Try finding by Message Key first
                var item = clayConfig.getItemByMessageKey(itemKey);
                
                // 2. Fallback: Try finding by element ID if message key doesn't match
                if (!item) {
                    item = clayConfig.getItemById(itemKey);
                }
                
                if (item) {
                    shouldShow ? item.show() : item.hide();
                } else {
                    console.log("Error: Could not find item '" + itemKey + "' by Key or ID!");
                }
            });
        };
    }
    
    // Special handler for animationSensitivity that checks dependent toggles
    function toggleAnimationsSens() {
        var currentValue = parseInt(this.get(), 10);
        var shouldShow = currentValue !== 0; // Show when NOT 0
        
        // Only show items if their parent toggles are also in the correct state
        var quietTimeBool = clayConfig.getItemByMessageKey('quietTimeBool');
        var animationsBatt = clayConfig.getItemByMessageKey('animationsBatt');
        
        // Check if quietTime items should be shown (quietTimeBool must be enabled)
        var showQuietTimeItems = shouldShow && quietTimeBool && !!quietTimeBool.get();
        var quietTimeStart = clayConfig.getItemByMessageKey('quietTimeStart');
        var quietTimeEnd = clayConfig.getItemByMessageKey('quietTimeEnd');
        if (quietTimeStart) showQuietTimeItems ? quietTimeStart.show() : quietTimeStart.hide();
        if (quietTimeEnd) showQuietTimeItems ? quietTimeEnd.show() : quietTimeEnd.hide();
        
        // Check if animationsCustom should be shown (animationsBatt must be 3)
        var showAnimationsCustom = shouldShow && animationsBatt && parseInt(animationsBatt.get(), 10) === 3;
        var animationsCustom = clayConfig.getItemByMessageKey('animationsCustom');
        if (animationsCustom) showAnimationsCustom ? animationsCustom.show() : animationsCustom.hide();
        
        // Always show/hide the parent toggles based on animationSensitivity
        if (quietTimeBool) shouldShow ? quietTimeBool.show() : quietTimeBool.hide();
        if (animationsBatt) shouldShow ? animationsBatt.show() : animationsBatt.hide();
    }
    
    // Configuration for simple toggles
    var toggleConfigs = [
        {
            triggerKey: 'quietTimeBool',
            isBoolean: true,
            items: ['quietTimeStart', 'quietTimeEnd'],
            showWhen: function(value) { return value === true; }
        },
        {
            triggerKey: 'animationsBatt',
            isBoolean: false,
            items: ['animationsCustom'],
            showWhen: function(value) { return value === 3; }
        },
        {
            triggerKey: 'stepsBool',
            isBoolean: true,
            items: ['stepsGoalBool', 'stepsGoal'],
            showWhen: function(value) { return value === true; }
        },
        {
            triggerKey: 'donate',
            isBoolean: true,
            items: ['donation_block'],
            showWhen: function(value) { return value === true; }
        },
        {
            triggerKey: 'hardcodeRivalBool',
            isBoolean: true,
            items: ['BeatTeam'],
            showWhen: function(value) { return value === false; }
        }
    ];
    
    // Set up all toggles after build
    clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function () {
        // Set up simple toggles
        toggleConfigs.forEach(function(config) {
            var toggleItem = clayConfig.getItemByMessageKey(config.triggerKey);
            
            if (toggleItem) {
                var handler = createToggleHandler(config);
                handler.call(toggleItem);
                toggleItem.on('change', handler);
            } else {
                console.log("Error: Could not find '" + config.triggerKey + "' toggle.");
            }
        });
        
        // Set up animationSensitivity with special logic
        var animationsSenseToggle = clayConfig.getItemByMessageKey('animationSensitivity');
        if (animationsSenseToggle) {
            toggleAnimationsSens.call(animationsSenseToggle);
            animationsSenseToggle.on('change', toggleAnimationsSens);
        } else {
            console.log("Error: Could not find 'animationSensitivity' toggle.");
        }
    });
};