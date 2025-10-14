extends Node
## Example 1: Basic Time System
##
## This example demonstrates:
## - Basic time unit registration (seconds, minutes, hours, days)
## - Time formatting and display
## - Pause/resume functionality
## - Time scale manipulation
## - Setting specific time values


var time_tick: TimeTick


func _ready() -> void:
	print("\nEXAMPLE 1: Basic Time System\n")
	
	# Create and initialize TimeTick
	time_tick = TimeTick.new()
	time_tick.initialize(0.1)  # Every 0.1 second, a new tick will be triggered.
	
	# Build standard time hierarchy

	# 1 tick = 1 second (virtual second, not real), wraps 0-59
	time_tick.register_time_unit("second", "tick", 1, 60, 0)
	# 60 seconds = 1 minute, wraps 0-59
	time_tick.register_time_unit("minute", "second", 60, 60, 0)
	# 60 minutes = 1 hour, wraps 0-23
	time_tick.register_time_unit("hour", "minute", 60, 24, 0)
	# 24 hours = 1 day, starts at day 1, no wrap
	time_tick.register_time_unit("day", "hour", 24, -1, 1)
	
	# Connect to signals
	time_tick.time_unit_changed.connect(_on_time_unit_changed)
	
	print("Standard time system initialized!")
	print("Time units: second, minute, hour, day")
	print("Time scale: 50x for demonstration\n")
	
	time_tick.set_time_scale(50.0)
	
	# Demonstrate setting specific time
	await get_tree().create_timer(2.0).timeout
	print("\n--- Setting time to 23:58:45 ---")
	time_tick.set_time_units({
		"hour": 23,
		"minute": 58,
		"second": 45
	})
	
	# Demonstrate pause/resume
	await get_tree().create_timer(4.0).timeout
	print("\n--- Pausing time")
	time_tick.pause()
	
	await get_tree().create_timer(2.0).timeout
	print("--- Resuming time")
	time_tick.resume()
	
	# Demonstrate time scale changes
	await get_tree().create_timer(3.0).timeout
	print("\n--- Increasing time scale to 200x")
	time_tick.set_time_scale(200.0)


func _on_time_unit_changed(unit_name: String, new_value: int, old_value: int) -> void:
	# Print formatted time every minute
	if unit_name == "minute":
		var formatted = time_tick.get_formatted_time_padded(["hour", "minute", "second"], ":")
		var day = time_tick.get_time_unit("day")
		print("[Day %d] %s" % [day, formatted])
	
	# Special message when day changes
	elif unit_name == "day":
		print("\n🌅 NEW DAY! Day %d → Day %d\n" % [old_value, new_value])


func _exit_tree() -> void:
	if time_tick:
		time_tick.shutdown()
