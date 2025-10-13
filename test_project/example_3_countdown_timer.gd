extends Node
## Example 3: Countdown Timer
##
## This example demonstrates how to create a countdown timer that counts backwards.
## Useful for mission timers, event countdowns, or timed challenges.

var time_tick: TimeTick


func _ready() -> void:
	print("\nEXAMPLE 3: Countdown Timer")
	
	# Create a new TimeTick instance
	time_tick = TimeTick.new()
	
	# Initialize with 1 second per tick
	time_tick.initialize(1.0)
	
	# Create countdown from 60 seconds (negative step = countdown)
	# Parameters: (name, parent, threshold, step_amount=-1, starting_value=60)
	time_tick.register_time_unit("countdown_second", "tick", 60, -1, 60)
	
	# Connect to countdown callback
	time_tick.time_unit_changed.connect(_on_countdown_changed)
	
	print("Countdown timer started from 60 seconds!")
	print("Watch it count down to zero...")


func _on_countdown_changed(unit_name: String, new_value: int, _old_value: int) -> void:
	if unit_name == "countdown_second":
		print("Countdown: %d seconds remaining" % new_value)
		
		if new_value <= 0:
			print("COUNTDOWN COMPLETE!")
			time_tick.pause()


func _exit_tree() -> void:
	if time_tick:
		time_tick.shutdown()
		print("TimeTick system shut down cleanly")
