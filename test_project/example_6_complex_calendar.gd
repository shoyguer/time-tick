extends Node
## Example 6: Complex Calendar System
##
## This example demonstrates the power of the new tracking system:
## Both "month" and "year" can track "day" with different trigger counts.
## - Month increments every 30 days
## - Year increments every 365 days
## This was impossible with the old parent-child system!

var time_tick: TimeTick


func _ready() -> void:
	print("\nEXAMPLE 6: Complex Calendar System")
	
	# Create a new TimeTick instance
	time_tick = TimeTick.new()
	
	# Initialize with tick duration (0.1 seconds = very fast for testing)
	time_tick.initialize(0.1)
	
	# Build complex time hierarchy
	# The key innovation: Both month AND year track "day" independently!
	# Parameters: (unit_name, tracked_unit, trigger_count, step_amount, max_value, starting_value)
	
	time_tick.register_time_unit("second", "tick", 1, 1, 60)           # 1 tick = 1 second, wraps at 60
	time_tick.register_time_unit("minute", "second", 60, 1, 60)        # 60 second overflows = 1 minute, wraps at 60
	time_tick.register_time_unit("hour", "minute", 60, 1, 24)          # 60 minute overflows = 1 hour, wraps at 24
	time_tick.register_time_unit("day", "hour", 24, 1, -1, 1)          # 24 hour overflows = 1 day, NO WRAP (unlimited), starts at 1
	
	# Here's the magic: Multiple units tracking the same parent with different counts
	# Day doesn't wrap (max_value=-1), so month and year count DAY INCREMENTS (not overflows)
	time_tick.register_time_unit("month", "day", 30, 1, 12, 1)         # Every 30 day increments = 1 month, wraps at 12, starts at 1
	time_tick.register_time_unit("year", "day", 365, 1, -1, 2024)      # Every 365 day increments = 1 year, no wrap, starts at 2024
	
	# Speed things up for demonstration
	time_tick.set_time_scale(10.0)  # 10x speed
	
	# Connect to signals for updates
	time_tick.tick_updated.connect(_on_tick_updated)
	time_tick.time_unit_changed.connect(_on_time_unit_changed)
	
	print("Complex calendar initialized!")
	print("Key feature: Month increments every 30 days, Year every 365 days")
	print("Both track 'day' independently - this is the new power!")
	print("Time scale set to 10x for faster demo")
	print("Watch how day counts up while both month and year increment separately...")


func _on_tick_updated(_current_tick: int) -> void:
	# Only print every 50 ticks to reduce spam
	if _current_tick % 50 == 0:
		var formatted := time_tick.get_formatted_time(
			"Year {year}, Month {month}, Day {day} - {hour}:{minute}:{second}"
		)
		print("Time: %s" % formatted)


func _on_time_unit_changed(unit_name: String, new_value: int, old_value: int) -> void:
	# Print important changes
	if unit_name in ["day", "month", "year"]:
		print("  → %s changed: %d -> %d" % [unit_name.capitalize(), old_value, new_value])
		
		# Show current day count when month or year changes
		if unit_name in ["month", "year"]:
			var current_day = time_tick.get_time_unit("day")
			print("    (Day is currently at: %d)" % current_day)


func _exit_tree() -> void:
	if time_tick:
		time_tick.shutdown()
