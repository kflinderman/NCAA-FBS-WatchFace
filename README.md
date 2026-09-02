# NCAA FBS Watchface

A Pebble watchface for following your favorite college football team, right on your wrist. Live scores, records, rankings, weather, and health stats, in a face built for gameday.

## Features

**Live scores & records**
Pulls real-time game data for your favorite FBS team from [CollegeFootballData.com](https://collegefootballdata.com). Current score, opponent, kickoff time, AP ranking, win/loss record, and postseason record updated automatically.

**Beat your rival**
Pick a team you want to see your favorite beat, a conference foe, a rival, whoever, and the watchface shows it alongside your own team.

**Countdown to kickoff**
A configurable countdown to your team's next game (or any custom date), so you always know how long until first snap.

**Weather**
Current conditions and temperature at a glance, with your choice of units.

**Health integration**
Step count, step goal progress, and heart rate, powered by Pebble Health.

**Battery & Bluetooth alerts**
Configurable vibration alerts for low/empty battery and Bluetooth disconnects, with quiet-hours support so you're not buzzed overnight.

**Fully customizable**
Every part of the display — what's shown, where, animation style and sensitivity, update frequency, quiet hours — is configurable from the in-app settings page. Includes an optional donation link if you'd like to support development.

**Efficient by design**
Score, records, and ranking data are synced smartly to stay light on battery and API usage: the watchface caches your last several favorite teams on-device, so switching between teams you follow doesn't cost a fresh API call, and syncing respects your quiet hours.

## Supported watches

Works on every Pebble platform: Pebble / Pebble Steel (aplite), Pebble Time / Time Steel (basalt), Pebble Time Round (chalk), Pebble 2 (diorite), Pebble Time 2 (emery), and later community-built hardware (gabbro, flint).

## Configuration

You'll need a free API key from [CollegeFootballData.com](https://collegefootballdata.com/key) to enable live score/record syncing. Enter it in the watchface's settings page (accessible from the Pebble app), along with your favorite team, rival, and display preferences.

## Building from source

This is a native Pebble project built with the [Rebble SDK](https://rebble.io/):

```bash
pebble build
pebble install --phone <your-phone-ip>   # or --emulator <platform>
```

The phone-side logic (season/score syncing, Clay configuration page) lives in `src/pkjs/`; the watch-side C code lives in `src/c/`.
