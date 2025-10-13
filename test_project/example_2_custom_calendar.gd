extends Node
## Example 2: Custom Fantasy Calendar
##
## This example demonstrates how to create a completely custom calendar system.
## Perfect for games with fictional worlds and unique time measurements.

var time_tick: TimeTick


func _ready() -> void:
	print("\nEXAMPLE 2: Custom Fantasy Calendar")
	
	# Create a new TimeTick instance
	time_tick = TimeTick.new()
	
	# Initialize with faster ticks for demonstration
	time_tick.initialize(0.5)
	
	# Build custom calendar hierarchy:
	# tick -> day -> week (7 days) -> month (4 weeks) -> season (3 months) -> year (4 seasons)
	# All starting at 1 for a natural calendar feel
	time_tick.register_time_unit("day", "tick", 7, 1, 1)         # 7 days per week
	time_tick.register_time_unit("week", "day", 4, 1, 1)         # 4 weeks per month
	time_tick.register_time_unit("month", "week", 3, 1, 1)       # 3 months per season
	time_tick.register_time_unit("season", "month", 4, 1, 1)     # 4 seasons per year
	time_tick.register_time_unit("year", "season", 999, 1, 1)    # Years (no upper limit)
	
	# Connect to calendar-specific callback
	time_tick.tick_updated.connect(_on_tick_updated)
	time_tick.time_unit_changed.connect(_on_time_unit_changed)
	
	print("Fantasy calendar initialized!")
	print("Structure: 7 days/week, 4 weeks/month, 3 months/season, 4 seasons/year")
	print("Starting at Year 1, Season 1, Month 1, Week 1, Day 1")


func _on_tick_updated(_current_tick: int) -> void:
	var year := time_tick.get_time_unit("year")
	var season := time_tick.get_time_unit("season")
	var month := time_tick.get_time_unit("month")
	var week := time_tick.get_time_unit("week")
	var day := time_tick.get_time_unit("day")
	
	print("Date: Year %d, Season %d, Month %d, Week %d, Day %d" % [year, season, month, week, day])


func _on_time_unit_changed(unit_name: String, new_value: int, old_value: int) -> void:
	print("  → %s changed: %d -> %d" % [unit_name.capitalize(), old_value, new_value])


func _exit_tree() -> void:
	if time_tick:
		time_tick.shutdown()
		print("TimeTick system shut down cleanly")
