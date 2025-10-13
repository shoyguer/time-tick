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
	
	# Build time hierarchy: tick -> second -> minute -> hour -> day
	# Parameters: (unit_name, parent_unit, threshold, step_amount, starting_value)
	time_tick.register_time_unit("second", "tick", 60, 5)       # Max 60, add 5 per tick
	time_tick.register_time_unit("minute", "second", 60)        # Max 60, add 1 per overflow
	time_tick.register_time_unit("hour", "minute", 60)          # Max 60, add 1 per overflow
	time_tick.register_time_unit("day", "hour", 24, 1, 1)       # Max 24, start at day 1
	
	# Connect to signals for updates
	time_tick.tick_updated.connect(_on_tick_updated)
	time_tick.time_unit_changed.connect(_on_time_unit_changed)
	
	print("Time system initialized!")
	print("Each tick adds 5 seconds (12 ticks = 1 minute)")
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
