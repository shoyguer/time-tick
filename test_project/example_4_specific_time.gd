extends Node
## Example 4: Start at Specific Time
##
## This example demonstrates how to set the initial time to a specific value.
## Useful for save/load systems or starting at a particular game time.

var time_tick: TimeTick


func _ready() -> void:
	print("\nEXAMPLE 4: Start at Specific Time")
	
	# Create a new TimeTick instance
	time_tick = TimeTick.new()
	
	# Initialize with 1 second per tick
	time_tick.initialize(1.0)
	
	# Register standard time units
	time_tick.register_time_unit("second", "tick", 60)
	time_tick.register_time_unit("minute", "second", 60)
	time_tick.register_time_unit("hour", "minute", 24)
	time_tick.register_time_unit("day", "hour", 999)
	
	# Method 1: Set individual values
	#time_tick.set_time_unit("day", 15)
	#time_tick.set_time_unit("hour", 14)
	#time_tick.set_time_unit("minute", 30)
	
	# Method 2: Set multiple values at once (recommended)
	time_tick.set_time_units({
		"day": 15,
		"hour": 14,
		"minute": 30,
		"second": 0
	})
	
	# Connect to callbacks
	time_tick.tick_updated.connect(_on_tick_updated)
	time_tick.time_unit_changed.connect(_on_time_unit_changed)
	
	print("Time system started at Day 15, 14:30:00")
	print("Perfect for loading saved game time!")


func _on_tick_updated(_current_tick: int) -> void:
	# Format time as "Day: 01:14:30:05" (day:hour:minute:second)
	var time_str := time_tick.get_formatted_time_padded(
		["day", "hour", "minute", "second"],
		":",
		2
	)
	print("Time: Day %s" % time_str)
	
	# Alternative: Custom format string with placeholders
	var formatted := time_tick.get_formatted_time("Day {day}, {hour}:{minute}:{second}")
	print("  └─ Formatted: %s" % formatted)


func _on_time_unit_changed(unit_name: String, new_value: int, old_value: int) -> void:
	print("  → %s changed: %d -> %d" % [unit_name.capitalize(), old_value, new_value])


func _exit_tree() -> void:
	if time_tick:
		time_tick.shutdown()
		print("TimeTick system shut down cleanly")
