extends Node
## Example 7: Special Event Timers
##
## This example demonstrates multiple independent timers tracking the same unit.
## Perfect for game events that trigger at specific intervals (e.g., enemy spawns, resource generation).

var time_tick: TimeTick


func _ready() -> void:
	print("\nEXAMPLE 7: Special Event Timers")
	
	# Create a new TimeTick instance
	time_tick = TimeTick.new()
	
	# Initialize with tick duration (0.1 seconds = very fast for testing)
	time_tick.initialize(0.1)
	
	# Build time hierarchy
	# Parameters: (unit_name, tracked_unit, trigger_count, step_amount, max_value, starting_value)
	
	time_tick.register_time_unit("second", "tick", 1, 1, 60)              # 1 tick = 1 second, wraps at 60
	
	# Multiple events tracking "second" overflows (not second values!)
	# Since second wraps at 60, these count how many times second wraps
	time_tick.register_time_unit("event_5sec", "second", 5, 1, -1)        # Every 5 second wraps (5 minutes)
	time_tick.register_time_unit("event_10sec", "second", 10, 1, -1)      # Every 10 second wraps (10 minutes)
	time_tick.register_time_unit("event_30sec", "second", 30, 1, -1)      # Every 30 second wraps (30 minutes)
	time_tick.register_time_unit("event_1800sec", "second", 1800, 1, -1)  # Every 1800 second wraps (30 hours!)
	
	# Speed things up for demonstration
	time_tick.set_time_scale(100.0)  # 100x speed
	
	# Connect to signals for updates
	time_tick.time_unit_changed.connect(_on_time_unit_changed)
	
	print("Event timers initialized!")
	print("All events track 'second' overflows (when second wraps from 59 to 0)")
	print("Time scale set to 100x for faster demo")
	print("")
	print("Expected triggers:")
	print("  - event_5sec: After 5 second wraps (5 minutes)")
	print("  - event_10sec: After 10 second wraps (10 minutes)")
	print("  - event_30sec: After 30 second wraps (30 minutes)")
	print("  - event_1800sec: After 1800 second wraps (30 hours)")
	print("")
	print("Watch the console for event triggers...")


func _on_time_unit_changed(unit_name: String, new_value: int, old_value: int) -> void:
	# Only print event triggers
	if unit_name.begins_with("event_"):
		var current_second = time_tick.get_time_unit("second")
		print("🎯 %s TRIGGERED! (value: %d -> %d, current second: %d)" % [unit_name.to_upper(), old_value, new_value, current_second])
		
		# Show some context
		match unit_name:
			"event_5sec":
				print("   → This event fires every 5 minutes in real-time")
			"event_10sec":
				print("   → This event fires every 10 minutes in real-time")
			"event_30sec":
				print("   → This event fires every 30 minutes in real-time")
			"event_1800sec":
				print("   → This event fires every 30 HOURS in real-time!")
				print("   → Perfect for daily quests, boss respawns, etc.")


func _exit_tree() -> void:
	if time_tick:
		time_tick.shutdown()
