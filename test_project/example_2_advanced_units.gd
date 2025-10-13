extends Node
## Example 2: Advanced Time Units
##
## This example demonstrates:
## - Custom calendars with non-standard time units
## - Variable step amounts (accelerated time progression)
## - Multiple independent event timers
## - Unit counters and overflow tracking
## - Max value wrapping behavior

var time_tick: TimeTick


func _ready() -> void:
	print("\n=== EXAMPLE 2: Advanced Time Units ===\n")
	
	# Create and initialize TimeTick
	time_tick = TimeTick.new()
	time_tick.initialize(0.05)
	
	# Custom fantasy calendar with non-standard time
	time_tick.register_time_unit("hour", "tick", 1, 28, 0)          # 28-hour day, wraps 0-27
	time_tick.register_time_unit("day", "hour", 28, 11, 1)          # 10-day week (1-10), starts at day 1
	time_tick.register_time_unit("week", "day", 10, 5, 1)           # 4-week month (1-4), starts at week 1
	time_tick.register_time_unit("month", "week", 4, 5, 1)          # 12-month year (1-4), starts at month 1
	time_tick.register_time_unit("year", "month", 4, -1, 1000)      # Starts at year 1000, no wrap
	
	# Set starting year after registration
	time_tick.set_time_unit_starting_value("year", 1000)
	
	# Event timers that track specific intervals
	time_tick.register_time_unit("harvest_timer", "hour", 28)       # Every 28 hours, no wrap
	time_tick.register_time_unit("market_day", "day", 15)           # Every 15 days, no wrap
	time_tick.register_time_unit("moon_cycle", "day", 28)           # Every 28 days, no wrap
	
	# Connect to signals
	time_tick.time_unit_changed.connect(_on_time_unit_changed)
	
	print("Custom Fantasy Calendar Initialized!")
	print("- 28 hours per day")
	print("- 10 days per week")
	print("- 4 weeks per month")
	print("- 12 months per year")
	print("\nSpecial Events:")
	print("- Harvest: Every 28 hours")
	print("- Market Day: Every 15 days")
	print("- Moon Cycle: Every 28 days\n")
	
	time_tick.set_time_scale(20.0)
	time_tick.pause()
	
	# Query available units
	await get_tree().create_timer(0.1).timeout
	var unit_names = time_tick.get_time_unit_names()
	print("\n--- Available Time Units ---")
	for unit_name in unit_names:
		var value = time_tick.get_time_unit(unit_name)
		print("  %s: %d" % [unit_name, value])
	
	await get_tree().create_timer(0.5).timeout
	time_tick.resume()


func _on_time_unit_changed(unit_name: String, new_value: int, old_value: int) -> void:
	match unit_name:
		"week":
			print("\n📅 New Week! Week %d of Month %d\n" % [new_value, time_tick.get_time_unit("month")])
		
		"month":
			print("\n📆 New Month! Month %d of Year %d\n" % [new_value, time_tick.get_time_unit("year")])
		
		"year":
			print("\n🎆 NEW YEAR! Year %d → Year %d 🎆\n" % [old_value, new_value])
		
		"harvest_timer":
			var hour = time_tick.get_time_unit("hour")
			var day = time_tick.get_time_unit("day")
			var week = time_tick.get_time_unit("week")
			var month = time_tick.get_time_unit("month")
			var year = time_tick.get_time_unit("year")
			print("  🌾 Harvest Time! [Year %d, Month %d, Week %d, Day %d, Hour %d] (Harvest #%d)" % [year, month, week, day, hour, new_value])
		
		"market_day":
			var day = time_tick.get_time_unit("day")
			print("  🏪 Market Day! (Day %d, Market #%d)" % [day, new_value])
		
		"moon_cycle":
			print("  🌙 Full Moon Cycle Complete! (Cycle #%d)" % new_value)


func _exit_tree() -> void:
	if time_tick:
		time_tick.shutdown()
