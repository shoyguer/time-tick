extends Node
## Example 1: Standard Time System
##
## This example demonstrates a basic time system similar to real-world time.
## Each tick advances seconds, which overflow into minutes, then hours, then days.

var time_tick: TimeTick


func _ready() -> void:
	print("\nEXAMPLE 1: Standard Time System")
	
	# Create a new TimeTick instance
	time_tick = TimeTick.new()
	
	# Initialize with tick duration (0.5 seconds = faster for testing)
	time_tick.initialize(0.5)
	
	# Build time hierarchy using the new API
	# Parameters: (unit_name, tracked_unit, trigger_count, step_amount, max_value, starting_value)
	# Each unit tracks another unit and increments when trigger_count is reached
	
	# For wrapping units (max_value > 0): children count OVERFLOWS
	# For non-wrapping units (max_value <= 0): children count INCREMENTS
	
	time_tick.register_time_unit("second", "tick", 1, 15, 60)    # 1 tick = 1 second, wraps at 60
	time_tick.register_time_unit("minute", "second", 60, 15, 60)  # Every 60 second (regardless of step!), wraps at 60
	time_tick.register_time_unit("hour", "minute", 60, 12, 24)    # Every 60 minute, wraps at 24
	time_tick.register_time_unit("day", "hour", 24, 1, -1, 1)    # Every 24 hour, no wrap, start at 1
	
	# Connect to signals for updates
	time_tick.tick_updated.connect(_on_tick_updated)
	time_tick.time_unit_changed.connect(_on_time_unit_changed)
	
	print("Time system initialized!")
	print("Standard time: seconds -> minutes -> hours -> days")
	print("Watch the console for time updates...")


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
