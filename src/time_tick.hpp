#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/typed_array.hpp>

using namespace godot;

/// A highly modular time tick system with customizable time hierarchies.
///
/// Allows you to define custom time units with any names and relationships.
/// Examples:
///   - "minute" increases every 60 "seconds"
///   - "day" increases every 24 "hours"
///   - "lunar_cycle" increases every 29 "days"
///
/// Usage:
///   Ref<TimeTick> time_tick = memnew(TimeTick);
///   time_tick->initialize(1.0);  // 1 second per tick
///   time_tick->register_time_unit("minute", "tick", 60);  // 60 ticks = 1 minute
///   time_tick->register_time_unit("hour", "minute", 60, 1, 14);  // 60 minutes = 1 hour. Start at 14:00
///   time_tick->register_time_unit("day", "hour", 24, 1, 5);  // 24 hours = 1 day. Start at day 5
///   time_tick->set_time_units({{"hour", 14}, {"minute", 30}});  // Set to 14:30
///   time_tick->set_time_unit_step("minute", 5);  // Each tick adds 5 minutes instead of 1
///   time_tick->tick_updated.connect(callable);
class TimeTick : public RefCounted {
	GDCLASS(TimeTick, RefCounted)

public:
	/// Internal struct to represent a time unit
	struct TimeUnit {
		String name = "";
		int current_value = 0;
		String parent_unit = "";  // What unit increases this one
		int threshold = 1;  // Maximum value before overflow
		int step_amount = 1;  // How much to add/subtract per parent increment
		
		TimeUnit() = default;
		
		TimeUnit(const String &p_name, const String &p_parent, int p_threshold, int p_step = 1)
			: name(p_name), parent_unit(p_parent), threshold(p_threshold), step_amount(p_step) {}
	};

	TimeTick();
	~TimeTick();

	/// Initialize the TimeTick system
	void initialize(double tick_duration = 1.0);
	
	/// Register a new time unit in the hierarchy
	/// [param unit_name] The name of this time unit (e.g., "minute", "hour", "day")
	/// [param parent_unit] The unit that triggers this one (e.g., "tick", "minute", "hour")  
	/// [param threshold] Maximum value before overflow (e.g., 60 for seconds/minutes)
	/// [param step_amount] How much to add per parent increment (can be negative to count down)
	/// [param starting_value] Optional initial value for this time unit (default: 0)
	void register_time_unit(const String &unit_name, const String &parent_unit, int threshold = 1, int step_amount = 1, int starting_value = 0);
	
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
	
	/// Dictionary of registered time units { "unit_name": TimeUnit }
	Dictionary time_units;
	
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
	
	/// Update time units based on parent unit incrementing
	void _update_time_units(const String &parent_unit);
};

