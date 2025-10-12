extends Node
## Example usage of the new modular TimeTick system

func _ready() -> void:
	# Example 1: Standard time (seconds -> minutes -> hours -> days)
	example_standard_time()
	
	# Example 2: Custom calendar (days -> months -> years)
	# example_custom_calendar()
	
	# Example 3: Countdown timer
	# example_countdown()
	
	# Example 4: Start at specific time
	# example_start_at_specific_time()


## Example 1: Standard time progression
func example_standard_time() -> void:
	print("\n=== EXAMPLE 1: Standard Time ===")
	
	# Initialize with 1 second per tick
	TimeTick.initialize(0.5)
	
	# Build time hierarchy: tick -> second -> minute -> hour -> day
	# threshold = max value before overflow, step_amount = how much to add per parent increment
	TimeTick.register_time_unit("second", "tick", 60, 30)      # Max 60, add 5 per tick
	TimeTick.register_time_unit("minute", "second", 60)       # Max 60, add 1 per overflow
	TimeTick.register_time_unit("hour", "minute", 60)         # Max 60, add 1 per overflow
	TimeTick.register_time_unit("day", "hour", 24, 1, 21)     # Max 24, add 1 per overflow, start at day 21
	
	# Connect to signals
	TimeTick.get_instance().tick_updated.connect(_on_tick_standard)
	TimeTick.get_instance().time_unit_changed.connect(_on_time_changed)
	
	print("Time system initialized!")
	print("Each tick adds 5 seconds (12 ticks = 1 minute)")


## Example 2: Custom fantasy calendar
func example_custom_calendar() -> void:
	print("\n=== EXAMPLE 2: Custom Calendar ===")
	
	# Initialize
	TimeTick.initialize(0.5)  # 0.5 seconds per tick (faster)
	
	# Build custom calendar:
	# tick -> day -> week (7 days) -> month (4 weeks) -> season (3 months) -> year (4 seasons)
	# Start at Year 1, Season 1, Month 1, Week 1, Day 1 by passing starting_value
	TimeTick.register_time_unit("day", "tick", 1, 1, 1)
	TimeTick.register_time_unit("week", "day", 7, 1, 1)
	TimeTick.register_time_unit("month", "week", 4, 1, 1)
	TimeTick.register_time_unit("season", "month", 3, 1, 1)
	TimeTick.register_time_unit("year", "season", 4, 1, 1)
	
	TimeTick.get_instance().tick_updated.connect(_on_tick_calendar)


## Example 3: Countdown timer (negative step)
func example_countdown() -> void:
	print("\n=== EXAMPLE 3: Countdown Timer ===")
	
	# Initialize
	TimeTick.initialize(1.0)
	
	# Countdown from 60 seconds (using starting_value parameter)
	TimeTick.register_time_unit("countdown_second", "tick", 60, -1, 60)  # -1 step, start at 60
	
	TimeTick.get_instance().time_unit_changed.connect(_on_countdown_changed)
	
	print("Countdown starting from 60...")


## Example 4: Start at a specific time
func example_start_at_specific_time() -> void:
	print("\n=== EXAMPLE 4: Start at Specific Time ===")
	
	# Initialize
	TimeTick.initialize(1.0)
	
	# Register time units
	TimeTick.register_time_unit("second", "tick", 1)
	TimeTick.register_time_unit("minute", "second", 60)
	TimeTick.register_time_unit("hour", "minute", 60)
	TimeTick.register_time_unit("day", "hour", 24)
	
	# Method 1: Set individual values
	# TimeTick.set_time_unit("day", 15)
	# TimeTick.set_time_unit("hour", 14)
	# TimeTick.set_time_unit("minute", 30)
	
	# Method 2: Set multiple values at once (cleaner!)
	TimeTick.set_time_units({
		"day": 15,
		"hour": 14,
		"minute": 30,
		"second": 0
	})
	
	TimeTick.get_instance().tick_updated.connect(_on_tick_standard)
	
	print("Started at Day 15, 14:30:00")


## Callback for standard time ticks
func _on_tick_standard(_tick: int) -> void:
	var time_str := TimeTick.get_formatted_time_padded(["day", "hour", "minute", "second"], ":")
	print("Time: Day %s" % time_str)
	
	# Alternative formatting
	var formatted := TimeTick.get_formatted_time("Day {day}, {hour}:{minute}:{second}")
	print("Formatted: %s" % formatted)


## Callback for calendar ticks
func _on_tick_calendar(_tick: int) -> void:
	var year := TimeTick.get_time_unit("year")
	var season := TimeTick.get_time_unit("season")
	var month := TimeTick.get_time_unit("month")
	var week := TimeTick.get_time_unit("week")
	var day := TimeTick.get_time_unit("day")
	
	print("Date: Year %d, Season %d, Month %d, Week %d, Day %d" % [year, season, month, week, day])


## Callback for time unit changes
func _on_time_changed(unit_name: String, new_value: int, old_value: int) -> void:
	print("  → %s changed: %d -> %d" % [unit_name.capitalize(), old_value, new_value])


## Callback for countdown changes
func _on_countdown_changed(unit_name: String, new_value: int, _old_value: int) -> void:
	if unit_name == "countdown_second":
		print("Countdown: %d seconds remaining" % new_value)
		
		if new_value <= 0:
			print("COUNTDOWN COMPLETE!")
			TimeTick.pause()


## Control functions you can call
func speed_up() -> void:
	TimeTick.set_time_scale(2.0)  # Double speed
	print("Time speed: 2x")


func slow_down() -> void:
	TimeTick.set_time_scale(0.5)  # Half speed
	print("Time speed: 0.5x")


func toggle_pause() -> void:
	TimeTick.toggle_pause()
	print("Paused: %s" % TimeTick.is_paused())
