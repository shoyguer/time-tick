extends Node
## Example 3: Complex Time Units
##
## This example demonstrates:
## - Complex time units (multiple conditions that must be met simultaneously)
## - Sidereal day (23h 56m 4s) vs Solar day (24h)
## - Specific time-based events (dawn, noon, dusk, midnight)
## - Lunar cycle tracking
## - Working with >= triggers (not exact matches)


var time_tick: TimeTick


func _ready() -> void:
	print("\nEXAMPLE 3: Complex Time Units\n")
	
	# Create and initialize TimeTick
	time_tick = TimeTick.new()
	time_tick.initialize(0.05)
	
	# Wraps 0-59
	time_tick.register_time_unit("second", "tick", 1, 60, 0)
	# 60 seconds = 1 minute, wraps 0-59
	time_tick.register_time_unit("minute", "second", 60, 60, 0)
	# 60 minutes = 1 hour, wraps 0-23
	time_tick.register_time_unit("hour", "minute", 60, 24, 0)
	
	# Fast seconds for demo
	time_tick.set_time_unit_step("second", 10)
	
	# No wrap
	time_tick.register_time_unit("solar_day", "hour", 24)
	
	# COMPLEX UNIT: Sidereal day (23h 56m 4s)
	# Triggers when hour >= 23 AND minute >= 56 AND second >= 4
	var sidereal_conditions = {
		"hour": 23,
		"minute": 56,
		"second": 4
	}
    # Search for sidereal day to understand what it is.
	time_tick.register_complex_time_unit("sidereal_day", sidereal_conditions)
	
	# COMPLEX UNITS: Time-of-day events
	var dawn_time = {"hour": 6, "minute": 0, "second": 0}
	var noon_time = {"hour": 12, "minute": 0, "second": 0}
	var dusk_time = {"hour": 18, "minute": 0, "second": 0}
	var midnight_time = {"hour": 0, "minute": 0, "second": 0}
	
	time_tick.register_complex_time_unit("dawn_event", dawn_time)
	time_tick.register_complex_time_unit("noon_event", noon_time)
	time_tick.register_complex_time_unit("dusk_event", dusk_time)
	time_tick.register_complex_time_unit("midnight_event", midnight_time)
	
	# COMPLEX UNIT: Lunar synodic month (29d 12h 44m 3s)
	# Note: This would require a 'day' unit to work properly
	# Wrap 0-29 for demo
	time_tick.register_time_unit("day", "hour", 24, 30, 0)
	var lunar_month_conditions = {
		"day": 29,
		"hour": 12,
		"minute": 44,
		"second": 3
	}
	time_tick.register_complex_time_unit("lunar_month", lunar_month_conditions)
	
	# Connect to signals
	time_tick.time_unit_changed.connect(_on_time_unit_changed)
	
	print("Complex Time Units Demonstration")
	print("\nStandard Units:")
	print("  - Solar Day: 24 hours")
	print("\nComplex Units:")
	print("  - Sidereal Day: Triggers at 23:56:04")
	print("  - Dawn: Triggers at 06:00:00")
	print("  - Noon: Triggers at 12:00:00")
	print("  - Dusk: Triggers at 18:00:00")
	print("  - Midnight: Triggers at 00:00:00")
	print("  - Lunar Month: Triggers at Day 29, 12:44:03")
	print("\nNote: Complex units trigger when ALL conditions are >= required values")
	print("      This means they work with any step amount!\n")
	
	time_tick.set_time_scale(500.0)
	
	# Start near a trigger point to demonstrate
	await get_tree().create_timer(1.0).timeout
	print("--- Jumping to 23:55:00 to watch sidereal day trigger\n")
	time_tick.set_time_units({
		"hour": 23,
		"minute": 55,
		"second": 0
	})


func _on_time_unit_changed(unit_name: String, new_value: int, old_value: int) -> void:
	var hour = time_tick.get_time_unit("hour")
	var minute = time_tick.get_time_unit("minute")
	var second = time_tick.get_time_unit("second")
	var day = time_tick.get_time_unit("day")
	
	match unit_name:
		"sidereal_day":
			var solar = time_tick.get_time_unit("solar_day")
			print("\n⭐ SIDEREAL DAY COMPLETE! (%02d:%02d:%02d)" % [hour, minute, second])
			print("   Sidereal: %d → %d" % [old_value, new_value])
			print("   Solar Days: %d (for comparison)" % solar)
			print("   → %d more sidereal day(s) than solar days\n" % (new_value - solar))
		
		"solar_day":
			var sidereal = time_tick.get_time_unit("sidereal_day")
			print("\n☀️  SOLAR DAY COMPLETE! (24:00:00)")
			print("   Solar: %d → %d" % [old_value, new_value])
			print("   Sidereal Days: %d (for comparison)" % sidereal)
			print("   → After 365 days, sidereal days will be 1 ahead!\n")
		
		"dawn_event":
			print("\n🌅 DAWN! (%02d:%02d:%02d)" % [hour, minute, second])
			print("   → The sun rises, day begins\n")
		
		"noon_event":
			print("\n☀️  NOON! (%02d:%02d:%02d)" % [hour, minute, second])
			print("   → The sun is at its peak\n")
		
		"dusk_event":
			print("\n🌆 DUSK! (%02d:%02d:%02d)" % [hour, minute, second])
			print("   → The sun sets, night begins\n")
		
		"midnight_event":
			print("\n🌙 MIDNIGHT! (%02d:%02d:%02d)" % [hour, minute, second])
			print("   → The witching hour, deepest night\n")
		
		"lunar_month":
			print("\n🌕 LUNAR MONTH COMPLETE! (Day %d at %02d:%02d:%02d)" % [day, hour, minute, second])
			print("   → Month: %d → %d" % [old_value, new_value])
			print("   → One synodic month (29d 12h 44m 3s) has passed\n")
		
		"minute":
			# Print current time every 10 minutes to show progression
			if minute % 10 == 0:
				var formatted = "%02d:%02d:%02d" % [hour, minute, second]
				var solar = time_tick.get_time_unit("solar_day")
				var sidereal = time_tick.get_time_unit("sidereal_day")
				print("[Day %d] %s | Solar: %d | Sidereal: %d" % [day, formatted, solar, sidereal])


func _exit_tree() -> void:
	if time_tick:
		time_tick.shutdown()
