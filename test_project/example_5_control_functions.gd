extends Node
## Example 5: Control Functions
##
## This example demonstrates how to control the time system:
## - Speed up / slow down time
## - Pause / resume
## - Reset the system
## - Get tick progress

var time_tick: TimeTick


func _ready() -> void:
	print("\nEXAMPLE 5: Control Functions")
	
	# Create and initialize TimeTick
	time_tick = TimeTick.new()
	time_tick.initialize(1.0)
	
	# Register simple time units
	time_tick.register_time_unit("second", "tick", 60)
	time_tick.register_time_unit("minute", "second", 60)
	
	time_tick.tick_updated.connect(_on_tick_updated)
	
	print("Time system initialized!")
	print("Use the control functions to manipulate time:")
	print("  - speed_up() : 2x speed")
	print("  - slow_down() : 0.5x speed")
	print("  - toggle_pause() : pause/resume")
	print("  - reset_time() : reset to zero")
	
	# Demonstrate automatic controls with timers
	await get_tree().create_timer(3.0).timeout
	speed_up()
	
	await get_tree().create_timer(3.0).timeout
	slow_down()
	
	await get_tree().create_timer(3.0).timeout
	toggle_pause()
	
	await get_tree().create_timer(2.0).timeout
	toggle_pause()
	
	await get_tree().create_timer(2.0).timeout
	reset_time()


func _on_tick_updated(current_tick: int) -> void:
	var progress := time_tick.get_tick_progress()
	var time_str := time_tick.get_formatted_time_padded(["minute", "second"], ":", 2)
	print("Tick %d | Time: %s | Progress: %.2f" % [current_tick, time_str, progress])


## Speed up time to 2x normal speed
func speed_up() -> void:
	if time_tick:
		time_tick.set_time_scale(2.0)
		print("\n[CONTROL] Time speed: 2x\n")


## Slow down time to 0.5x normal speed
func slow_down() -> void:
	if time_tick:
		time_tick.set_time_scale(0.5)
		print("\n[CONTROL] Time speed: 0.5x\n")


## Toggle pause/resume of the time system
func toggle_pause() -> void:
	if time_tick:
		time_tick.toggle_pause()
		var status := "PAUSED" if time_tick.is_paused() else "RESUMED"
		print("\n[CONTROL] %s\n" % status)


## Reset the time system back to zero
func reset_time() -> void:
	if time_tick:
		time_tick.reset()
		print("\n[CONTROL] Time system reset to zero\n")


func _exit_tree() -> void:
	if time_tick:
		time_tick.shutdown()
		print("TimeTick system shut down cleanly")
