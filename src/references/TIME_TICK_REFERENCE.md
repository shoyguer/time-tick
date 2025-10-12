# TimeTick System - Quick Reference

## Overview
The TimeTick system is a highly modular time management system that allows you to create custom time hierarchies with any units and relationships.

## Basic Setup

```gdscript
# Initialize the system (1 second per tick)
TimeTick.initialize(1.0)

# Register time units
TimeTick.register_time_unit("minute", "tick", 60)    # 60 ticks = 1 minute
TimeTick.register_time_unit("hour", "minute", 60)    # 60 minutes = 1 hour
TimeTick.register_time_unit("day", "hour", 24)       # 24 hours = 1 day

# Connect to signals
TimeTick.get_instance().tick_updated.connect(_on_tick)
TimeTick.get_instance().time_unit_changed.connect(_on_time_changed)
```

## Key Concepts

### Time Units
Each time unit has:
- **name**: Identifier (e.g., "minute", "hour", "day")
- **parent_unit**: What triggers this unit (e.g., "tick", "minute")
- **threshold**: How many parent units needed to increment by 1
- **step_amount**: How much to add per parent increment (can be negative)

### Example Hierarchies

#### Standard Time
```gdscript
tick -> second (60) -> minute (60) -> hour (24) -> day
```

#### Custom Calendar
```gdscript
tick -> day (7) -> week (4) -> month (12) -> year
```

#### Game Example
```gdscript
tick -> day (7) -> month (6) -> year
# Where: 7 days per month, 6 months per year
```

## Core Functions

### Initialization
```gdscript
TimeTick.initialize(tick_duration: float)
# tick_duration: Real-time seconds between ticks
```

### Register Time Unit
```gdscript
TimeTick.register_time_unit(unit_name: String, parent_unit: String, threshold: int, step_amount: int = 1, starting_value: int = 0)
# unit_name: Name of this unit (e.g., "hour")
# parent_unit: What increments this (e.g., "minute")
# threshold: How many parent units = 1 of this unit
# step_amount: How much to add per parent tick (default: 1, use negative for countdown)
# starting_value: Initial value for this unit (default: 0)
```

### Get/Set Time Values
```gdscript
# Get current value
var hours = TimeTick.get_time_unit("hour")

# Set value directly
TimeTick.set_time_unit("hour", 12)

# Set multiple values at once
TimeTick.set_time_units({"day": 5, "hour": 14, "minute": 30})

# Change step amount
TimeTick.set_time_unit_step("minute", 5)  # Add 5 minutes per tick instead of 1
```

### Control
```gdscript
TimeTick.pause()           # Pause time
TimeTick.resume()          # Resume time
TimeTick.toggle_pause()    # Toggle pause state
TimeTick.set_time_scale(2.0)  # Speed multiplier (2.0 = double speed)
TimeTick.reset()           # Reset all units to 0
```

### Formatting
```gdscript
# Custom format with placeholders
var time = TimeTick.get_formatted_time("Day {day}, {hour}:{minute}")
# Output: "Day 5, 14:30"

# Padded format
var time = TimeTick.get_formatted_time_padded(["hour", "minute", "second"], ":")
# Output: "14:30:05"
```

## Advanced Examples

### Starting at Specific Time
```gdscript
# Method 1: Pass starting_value when registering
TimeTick.initialize(1.0)
TimeTick.register_time_unit("hour", "tick", 1, 1, 14)     # Start at 14:00
TimeTick.register_time_unit("minute", "hour", 60, 1, 30)  # Start at :30

# Method 2: Set after registration
TimeTick.register_time_unit("hour", "tick", 1)
TimeTick.register_time_unit("minute", "hour", 60)
TimeTick.set_time_unit("hour", 14)
TimeTick.set_time_unit("minute", 30)

# Method 3: Set multiple at once (cleanest!)
TimeTick.register_time_unit("day", "tick", 1)
TimeTick.register_time_unit("hour", "day", 24)
TimeTick.register_time_unit("minute", "hour", 60)
TimeTick.set_time_units({"day": 5, "hour": 14, "minute": 30})
# Starts at: Day 5, 14:30
```

### Fast-Forward Time
```gdscript
# Each tick adds 10 minutes instead of 1
TimeTick.register_time_unit("minute", "tick", 1)
TimeTick.set_time_unit_step("minute", 10)
```

### Countdown Timer
```gdscript
# Negative step for countdown
TimeTick.register_time_unit("countdown", "tick", 60, -1)
TimeTick.set_time_unit("countdown", 60)  # Start at 60
# Now it counts down: 60, 59, 58, ...
```

### Complex Calendar
```gdscript
TimeTick.initialize(1.0)

# Your custom calendar system with starting values
TimeTick.register_time_unit("day", "tick", 1, 1, 1)       # Start at day 1
TimeTick.register_time_unit("month", "day", 7, 1, 3)      # Start at month 3, 7-day months
TimeTick.register_time_unit("year", "month", 6, 1, 2025)  # Start at year 2025, 6-month years
# Current time: Year 2025, Month 3, Day 1
```

### Mixed Units
```gdscript
# Combine different step amounts
TimeTick.register_time_unit("second", "tick", 1, 1)      # +1 per tick
TimeTick.register_time_unit("minute", "second", 60, 1)   # +1 per 60 seconds
TimeTick.register_time_unit("energy", "tick", 1, -2)     # -2 per tick (drains)
TimeTick.register_time_unit("mana", "second", 5, 3)      # +3 every 5 seconds
```

## Signals

### tick_updated
Emitted every tick.
```gdscript
func _on_tick(current_tick: int) -> void:
    print("Tick: %d" % current_tick)
```

### time_unit_changed
Emitted when any time unit changes value.
```gdscript
func _on_time_changed(unit_name: String, new_value: int, old_value: int) -> void:
    if unit_name == "hour":
        print("Hour changed from %d to %d" % [old_value, new_value])
```

## Common Patterns

### Real-time to Game-time
```gdscript
# 1 real second = 1 game minute
TimeTick.initialize(1.0)
TimeTick.register_time_unit("minute", "tick", 1)
TimeTick.register_time_unit("hour", "minute", 60)
```

### Accelerated Time
```gdscript
# 1 real second = 5 game minutes
TimeTick.initialize(1.0)
TimeTick.register_time_unit("minute", "tick", 1, 5)  # step_amount = 5
TimeTick.register_time_unit("hour", "minute", 60)
```

### Resource Regeneration
```gdscript
# Health regenerates 1 point every 3 seconds
TimeTick.register_time_unit("health_regen", "tick", 3, 1)

func _on_time_changed(unit_name: String, new_value: int, _old_value: int) -> void:
    if unit_name == "health_regen":
        player.health += 1
        TimeTick.set_time_unit("health_regen", 0)  # Reset
```

## Tips

1. **Always initialize first**: Call `TimeTick.initialize()` before registering units
2. **Use meaningful names**: "minute", "day", "lunar_cycle" are better than "unit1", "unit2"
3. **Parent units must exist**: Register in order (tick -> second -> minute)
4. **Negative steps**: Use for countdowns or draining resources
5. **Time scale**: Use for game speed changes (pause menus, fast-forward)
6. **Signals**: Listen to `time_unit_changed` for specific unit updates

## Cleanup

```gdscript
# When done (e.g., changing scenes)
TimeTick.shutdown()
```
