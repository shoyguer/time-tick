class_name TimeTick
extends RefCounted
## A highly modular time tick system with customizable time hierarchies.
##
## Allows you to define custom time units with any names and relationships.
## Examples:
##   - "minute" increases every 60 "seconds"
##   - "day" increases every 24 "hours"
##   - "lunar_cycle" increases every 29 "days"
##
## Usage:
##   TimeTick.initialize(1.0)  # 1 second per tick
##   TimeTick.register_time_unit("minute", "tick", 60)  # 60 ticks = 1 minute
##   TimeTick.register_time_unit("hour", "minute", 60, 1, 14)  # 60 minutes = 1 hour. Start at 14:00
##   TimeTick.register_time_unit("day", "hour", 24, 1, 5)  # 24 hours = 1 day. Start at day 5
##   # Or set starting values after registration:
##   TimeTick.set_time_units({"hour": 14, "minute": 30})  # Set to 14:30
##   TimeTick.set_time_unit_step("minute", 5)  # Each tick adds 5 minutes instead of 1
##   TimeTick.get_instance().tick_updated.connect(_on_tick)

## Signal emitted whenever a tick passes
signal tick_updated(current_tick: int)
## Signal emitted when a time unit changes (e.g., minute, hour, day)
signal time_unit_changed(unit_name: String, new_value: int, old_value: int)

## Time in real-time seconds for each tick to update
static var tick_time: float = 1.0
## Current tick count
static var current_tick: int = 0
## Time scale multiplier (1.0 = normal speed, 2.0 = double speed, 0.5 = half speed)
static var time_scale: float = 1.0

## Dictionary of registered time units { "unit_name": TimeUnit }
static var _time_units: Dictionary = {}
## Accumulated delta time since last tick
static var _accumulated_time: float = 0.0
## Whether the tick system is paused
static var _is_paused: bool = false
## Singleton instance for signal emission
static var _instance: TimeTick = null
## Track last physics frame time for delta calculation
static var _last_physics_time: float = 0.0
## Whether the system has been initialized
static var _initialized: bool = false


## Internal class to represent a time unit
class TimeUnit:
	var name: String
	var current_value: int = 0
	var parent_unit: String = ""  # What unit increases this one
	var threshold: int = 1  # Maximum value before overflow
	var step_amount: int = 1  # How much to add/subtract per parent increment
	
	func _init(p_name: String, p_parent: String, p_threshold: int, p_step: int = 1) -> void:
		name = p_name
		parent_unit = p_parent
		threshold = p_threshold
		step_amount = p_step


## Initialize the TimeTick system
static func initialize(tick_duration: float = 1.0) -> void:
	tick_time = tick_duration
	current_tick = 0
	_accumulated_time = 0.0
	_is_paused = false
	time_scale = 1.0
	_initialized = true
	_time_units.clear()
	
	# Create singleton instance if it doesn't exist
	if _instance == null:
		_instance = TimeTick.new()
	
	# Connect to SceneTree's physics_frame signal
	var tree := Engine.get_main_loop() as SceneTree
	if tree and not tree.physics_frame.is_connected(_on_physics_frame):
		tree.physics_frame.connect(_on_physics_frame)
		_last_physics_time = Time.get_ticks_msec() / 1000.0


## Register a new time unit in the hierarchy
## [param unit_name] The name of this time unit (e.g., "minute", "hour", "day")
## [param parent_unit] The unit that triggers this one (e.g., "tick", "minute", "hour")  
## [param threshold] Maximum value before overflow (e.g., 60 for seconds/minutes)
## [param step_amount] How much to add per parent increment (can be negative to count down)
## [param starting_value] Optional initial value for this time unit (default: 0)
static func register_time_unit(unit_name: String, parent_unit: String, threshold: int = 1, step_amount: int = 1, starting_value: int = 0) -> void:
	if unit_name.is_empty():
		push_error("TimeTick: Unit name cannot be empty")
		return
	
	if threshold <= 0:
		push_error("TimeTick: Threshold must be positive")
		return
	
	var time_unit := TimeUnit.new(unit_name, parent_unit, threshold, step_amount)
	time_unit.current_value = starting_value
	_time_units[unit_name] = time_unit


## Unregister a time unit
static func unregister_time_unit(unit_name: String) -> void:
	_time_units.erase(unit_name)


## Set the step amount for a time unit (how much it increases per parent tick)
static func set_time_unit_step(unit_name: String, step_amount: int) -> void:
	if _time_units.has(unit_name):
		_time_units[unit_name].step_amount = step_amount
	else:
		push_error("TimeTick: Time unit '%s' not found" % unit_name)


## Get the current value of a time unit
static func get_time_unit(unit_name: String) -> int:
	if _time_units.has(unit_name):
		return _time_units[unit_name].current_value
	return 0


## Set the value of a time unit directly
static func set_time_unit(unit_name: String, value: int) -> void:
	if _time_units.has(unit_name):
		var old_value: int = _time_units[unit_name].current_value
		_time_units[unit_name].current_value = value
		if _instance != null:
			_instance.time_unit_changed.emit(unit_name, value, old_value)
	else:
		push_error("TimeTick: Time unit '%s' not found" % unit_name)


## Set multiple time unit values at once
## [param values] Dictionary mapping unit names to their starting values (e.g., {"day": 5, "hour": 14, "minute": 30})
static func set_time_units(values: Dictionary) -> void:
	for unit_name in values.keys():
		set_time_unit(unit_name, values[unit_name])


## Get all registered time unit names
static func get_time_unit_names() -> Array[String]:
	var names: Array[String] = []
	for key in _time_units.keys():
		names.append(key)
	return names


## Get a formatted time string (e.g., "Day 5, 14:30")
## [param format_string] Use {unit_name} placeholders (e.g., "Day {day}, {hour}:{minute}")
static func get_formatted_time(format_string: String) -> String:
	var result := format_string
	for unit_name in _time_units.keys():
		var value: int = _time_units[unit_name].current_value
		var placeholder: String = "{" + unit_name + "}"
		result = result.replace(placeholder, str(value))
	return result


## Get a formatted time string with padding (e.g., "05:03" for 5 hours 3 minutes)
## [param units] Array of unit names to include (e.g., ["hour", "minute"])
## [param separator] String to separate values (e.g., ":")
## [param padding] Minimum digits per value (e.g., 2 for "05")
static func get_formatted_time_padded(units: Array[String], separator: String = ":", padding: int = 2) -> String:
	var parts: Array[String] = []
	for unit_name in units:
		if _time_units.has(unit_name):
			var value: int = _time_units[unit_name].current_value
			parts.append(str(value).pad_zeros(padding))
		else:
			parts.append("00")
	return separator.join(parts)


## Cleanup and disconnect from signals
static func shutdown() -> void:
	var tree := Engine.get_main_loop() as SceneTree
	if tree and tree.physics_frame.is_connected(_on_physics_frame):
		tree.physics_frame.disconnect(_on_physics_frame)
	
	_instance = null
	_initialized = false
	_time_units.clear()


## Internal callback for SceneTree's physics_frame signal
static func _on_physics_frame() -> void:
	if not _initialized:
		return
	
	var current_time := Time.get_ticks_msec() / 1000.0
	var delta := current_time - _last_physics_time
	_last_physics_time = current_time
	_process_tick(delta)


## Main tick processing function
static func _process_tick(delta: float) -> void:
	if _is_paused:
		return
	
	# Apply time scale
	var scaled_delta := delta * time_scale
	_accumulated_time += scaled_delta
	
	# Process all ticks that should have occurred
	while _accumulated_time >= tick_time:
		_accumulated_time -= tick_time
		current_tick += 1
		
		# Update time hierarchy
		_update_time_units("tick")
		
		# Emit signal through singleton instance
		if _instance != null:
			_instance.tick_updated.emit(current_tick)


## Update time units based on parent unit incrementing
static func _update_time_units(parent_unit: String) -> void:
	# Find all units that depend on this parent
	for unit_name in _time_units.keys():
		var time_unit: TimeUnit = _time_units[unit_name]
		
		if time_unit.parent_unit == parent_unit:
			var old_value := time_unit.current_value
			time_unit.current_value += time_unit.step_amount
			
			# Check if this unit has overflowed its threshold
			while time_unit.current_value >= time_unit.threshold:
				time_unit.current_value -= time_unit.threshold
				# Trigger child units that depend on this one
				_update_time_units(unit_name)
			
			# Handle negative values (countdown)
			while time_unit.current_value < 0:
				time_unit.current_value += time_unit.threshold
				# Trigger child units
				_update_time_units(unit_name)
			
			# Emit change signal
			if _instance != null and old_value != time_unit.current_value:
				_instance.time_unit_changed.emit(unit_name, time_unit.current_value, old_value)


## Pause the tick system
static func pause() -> void:
	_is_paused = true


## Resume the tick system
static func resume() -> void:
	_is_paused = false


## Toggle pause state
static func toggle_pause() -> void:
	_is_paused = not _is_paused


## Check if the system is paused
static func is_paused() -> bool:
	return _is_paused


## Reset the tick system and all time units to zero
static func reset() -> void:
	current_tick = 0
	_accumulated_time = 0.0
	for unit_name in _time_units.keys():
		_time_units[unit_name].current_value = 0


## Set the time scale (speed multiplier)
static func set_time_scale(scale: float) -> void:
	time_scale = max(0.0, scale)


## Get the current tick count
static func get_current_tick() -> int:
	return current_tick


## Get the progress to the next tick (0.0 to 1.0)
static func get_tick_progress() -> float:
	if tick_time <= 0.0:
		return 0.0
	return clampf(_accumulated_time / tick_time, 0.0, 1.0)


## Get the singleton instance (for signal connections)
static func get_instance() -> TimeTick:
	if _instance == null:
		_instance = TimeTick.new()
	return _instance


## Check if the system has been initialized
static func is_initialized() -> bool:
	return _initialized
