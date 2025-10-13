#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/typed_array.hpp>

using namespace godot;

/// A highly modular time tick system with customizable time hierarchies.
///
/// Allows you to define custom time units with any names and relationships.
/// Each unit tracks another unit and increments when that unit reaches a certain count.
/// Examples:
///   - "minute" tracks "tick" and increments every 60 ticks
///   - "hour" tracks "minute" and increments every 60 minutes
///   - "month" tracks "day" and increments every 30 days
///   - "year" tracks "day" and increments every 365 days (independent of month!)
///
/// Usage:
///   Ref<TimeTick> time_tick = memnew(TimeTick);
///   time_tick->initialize(1.0);  // 1 second per tick
///   time_tick->register_time_unit("second", "tick", 1);  // 1 tick = 1 second
///   time_tick->register_time_unit("minute", "second", 60, 1, 60);  // 60 seconds = 1 minute, wraps at 60
///   time_tick->register_time_unit("hour", "minute", 60, 1, 24, 14);  // 60 minutes = 1 hour, wraps at 24, starts at 14
///   time_tick->register_time_unit("day", "hour", 24, 1, -1, 1);  // 24 hours = 1 day, no wrap, starts at 1
///   time_tick->register_time_unit("month", "day", 30, 1, 12);  // 30 days = 1 month, wraps at 12
///   time_tick->register_time_unit("year", "day", 365);  // 365 days = 1 year, no wrap
///   time_tick->set_time_unit_step("minute", 5);  // Each tick adds 5 minutes instead of 1
///   time_tick->tick_updated.connect(callable);
class TimeTick : public RefCounted {
	GDCLASS(TimeTick, RefCounted)

public:
	/// Internal struct to represent a time unit
	struct TimeUnit {
		String name = "";
		int current_value = 0;
		String tracked_unit = "";  // What unit we're counting (e.g., "day" tracks itself)
		int trigger_count = 1;  // How many of tracked_unit before we increment
		int step_amount = 1;  // How much to add when triggered
		int max_value = -1;  // Maximum value before wrapping to 0 (-1 = no wrap)
		
		TimeUnit() = default;
		
		TimeUnit(const String &p_name, const String &p_tracked, int p_trigger_count, int p_step = 1, int p_max = -1)
			: name(p_name), tracked_unit(p_tracked), trigger_count(p_trigger_count), step_amount(p_step), max_value(p_max) {}
	};

	TimeTick();
	~TimeTick();

	/// Initialize the TimeTick system
	void initialize(double tick_duration = 1.0);
	
	/// Register a new time unit in the hierarchy
	/// [param unit_name] The name of this time unit (e.g., "minute", "hour", "month", "year")
	/// [param tracked_unit] The unit we're counting (e.g., "minute" tracks "tick", "month" tracks "day")
	/// [param trigger_count] How many of tracked_unit before incrementing (e.g., 60 ticks = 1 minute, 30 days = 1 month)
	/// [param step_amount] How much to add when triggered (can be negative to count down)
	/// [param max_value] Maximum value before wrapping to 0 (e.g., 24 for hours, 12 for months, -1 for unlimited)
	/// [param starting_value] Optional initial value for this time unit (default: 0)
	void register_time_unit(const String &unit_name, const String &tracked_unit, int trigger_count = 1, int step_amount = 1, int max_value = -1, int starting_value = 0);
	
	/// Unregister a time unit
	void unregister_time_unit(const String &unit_name);
	
	/// Set the step amount for a time unit (how much it increases per parent tick)
	void set_time_unit_step(const String &unit_name, int step_amount);
	
	/// Get the current value of a time unit
	int get_time_unit(const String &unit_name) const;
	
	/// Set the value of a time unit directly
	void set_time_unit(const String &unit_name, int value);
	
	/// Set multiple time unit values at once
	/// @param values Dictionary mapping unit names to their starting values (e.g., {"day": 5, "hour": 14, "minute": 30})
	void set_time_units(const Dictionary &values);
	
	/// Get all registered time unit names
	TypedArray<String> get_time_unit_names() const;
	
	/// Get a formatted time string (e.g., "Day 5, 14:30")
	/// @param format_string Use {unit_name} placeholders (e.g., "Day {day}, {hour}:{minute}")
	String get_formatted_time(const String &format_string) const;
	
	/// Get a formatted time string with padding (e.g., "05:03" for 5 hours 3 minutes)
	/// @param units Array of unit names to include (e.g., ["hour", "minute"])
	/// @param separator String to separate values (e.g., ":")
	/// @param padding Minimum digits per value (e.g., 2 for "05")
	String get_formatted_time_padded(const TypedArray<String> &units, const String &separator = ":", int padding = 2) const;
	
	/// Cleanup and disconnect from signals
	void shutdown();
	
	/// Pause the tick system
	void pause();
	
	/// Resume the tick system
	void resume();
	
	/// Toggle pause state
	void toggle_pause();
	
	/// Check if the system is paused
	bool is_paused() const;
	
	/// Reset the tick system and all time units to zero
	void reset();
	
	/// Set the time scale (speed multiplier)
	void set_time_scale(double scale);
	
	/// Get the current time scale
	double get_time_scale() const;
	
	/// Get the current tick count
	int get_current_tick() const;
	
	/// Get the progress to the next tick (0.0 to 1.0)
	double get_tick_progress() const;
	
	/// Check if the system has been initialized
	bool is_initialized() const;

protected:
	static void _bind_methods();

private:
	/// Time in real-time seconds for each tick to update
	double tick_time = 1.0;
	
	/// Current tick count
	int current_tick = 0;
	
	/// Time scale multiplier (1.0 = normal speed, 2.0 = double speed, 0.5 = half speed)
	double time_scale = 1.0;
	
	/// Dictionary of registered time units { "unit_name": Dictionary }
	Dictionary time_units;
	
	/// Track how many times each unit has counted its tracked unit (for trigger counting)
	/// Key is the unit doing the tracking (e.g., "minute" tracks how many "seconds" it has seen)
	Dictionary unit_counters;
	
	/// Accumulated delta time since last tick
	double accumulated_time = 0.0;
	
	/// Whether the tick system is paused
	bool paused = false;
	
	/// Track last physics frame time for delta calculation
	double last_physics_time = 0.0;
	
	/// Whether the system has been initialized
	bool initialized = false;
	
	/// Connected to physics_frame signal
	bool connected_to_physics = false;
	
	/// Internal callback for SceneTree's physics_frame signal
	void _on_physics_frame();
	
	/// Main tick processing function
	void _process_tick(double delta);
	
	/// Increment a specific unit and check what it triggers
	void _increment_unit(const String &unit_name);
};

