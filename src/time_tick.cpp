#include "time_tick.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

TimeTick::TimeTick() {
	// Constructor - initialize default values (already done in header with default member initializers)
}

TimeTick::~TimeTick() {
	// Cleanup on destruction
	shutdown();
}

void TimeTick::initialize(double tick_duration) {
	tick_time = tick_duration;
	current_tick = 0;
	accumulated_time = 0.0;
	paused = false;
	time_scale = 1.0;
	initialized = true;
	time_units.clear();
	unit_counters.clear();
	
	// Connect to SceneTree's physics_frame signal
	SceneTree *tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
	if (tree && !connected_to_physics) {
		Callable callback = callable_mp(this, &TimeTick::_on_physics_frame);
		if (!tree->is_connected("physics_frame", callback)) {
			tree->connect("physics_frame", callback);
			connected_to_physics = true;
		}
		last_physics_time = Time::get_singleton()->get_ticks_msec() / 1000.0;
	}
}

void TimeTick::register_time_unit(const String &unit_name, const String &tracked_unit, int trigger_count, int step_amount, int max_value, int starting_value) {
	if (unit_name.is_empty()) {
		UtilityFunctions::push_error("TimeTick: Unit name cannot be empty");
		return;
	}
	
	if (trigger_count <= 0) {
		UtilityFunctions::push_error("TimeTick: Trigger count must be positive");
		return;
	}
	
	// Create a Dictionary to store TimeUnit data
	Dictionary time_unit;
	time_unit["name"] = unit_name;
	time_unit["current_value"] = starting_value;
	time_unit["tracked_unit"] = tracked_unit;
	time_unit["trigger_count"] = trigger_count;
	time_unit["step_amount"] = step_amount;
	time_unit["max_value"] = max_value;
	
	time_units[unit_name] = time_unit;
	
	// Initialize counter for this unit
	if (!unit_counters.has(unit_name)) {
		unit_counters[unit_name] = 0;
	}
}

void TimeTick::unregister_time_unit(const String &unit_name) {
	time_units.erase(unit_name);
}

void TimeTick::set_time_unit_step(const String &unit_name, int step_amount) {
	if (time_units.has(unit_name)) {
		Dictionary time_unit = time_units[unit_name];
		time_unit["step_amount"] = step_amount;
		time_units[unit_name] = time_unit;
	} else {
		UtilityFunctions::push_error(vformat("TimeTick: Time unit '%s' not found", unit_name));
	}
}

int TimeTick::get_time_unit(const String &unit_name) const {
	if (time_units.has(unit_name)) {
		Dictionary time_unit = time_units[unit_name];
		return time_unit["current_value"];
	}
	return 0;
}

void TimeTick::set_time_unit(const String &unit_name, int value) {
	if (time_units.has(unit_name)) {
		Dictionary time_unit = time_units[unit_name];
		int old_value = time_unit["current_value"];
		time_unit["current_value"] = value;
		time_units[unit_name] = time_unit;
		
		// Emit signal
		emit_signal("time_unit_changed", unit_name, value, old_value);
	} else {
		UtilityFunctions::push_error(vformat("TimeTick: Time unit '%s' not found", unit_name));
	}
}

void TimeTick::set_time_units(const Dictionary &values) {
	Array keys = values.keys();
	for (int i = 0; i < keys.size(); i++) {
		String unit_name = keys[i];
		int value = values[unit_name];
		set_time_unit(unit_name, value);
	}
}

TypedArray<String> TimeTick::get_time_unit_names() const {
	TypedArray<String> names;
	Array keys = time_units.keys();
	for (int i = 0; i < keys.size(); i++) {
		names.append(keys[i]);
	}
	return names;
}

String TimeTick::get_formatted_time(const String &format_string) const {
	String result = format_string;
	Array keys = time_units.keys();
	
	for (int i = 0; i < keys.size(); i++) {
		String unit_name = keys[i];
		Dictionary time_unit = time_units[unit_name];
		int value = time_unit["current_value"];
		String placeholder = "{" + unit_name + "}";
		result = result.replace(placeholder, String::num_int64(value));
	}
	
	return result;
}

String TimeTick::get_formatted_time_padded(const TypedArray<String> &units, const String &separator, int padding) const {
	PackedStringArray parts;
	
	for (int i = 0; i < units.size(); i++) {
		String unit_name = units[i];
		if (time_units.has(unit_name)) {
			Dictionary time_unit = time_units[unit_name];
			int value = time_unit["current_value"];
			parts.append(String::num_int64(value).pad_zeros(padding));
		} else {
			parts.append("00");
		}
	}
	
	return separator.join(parts);
}

void TimeTick::shutdown() {
	// Disconnect from SceneTree's physics_frame signal
	SceneTree *tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
	if (tree && connected_to_physics) {
		Callable callback = callable_mp(this, &TimeTick::_on_physics_frame);
		if (tree->is_connected("physics_frame", callback)) {
			tree->disconnect("physics_frame", callback);
		}
		connected_to_physics = false;
	}
	
	initialized = false;
	time_units.clear();
}

void TimeTick::pause() {
	paused = true;
}

void TimeTick::resume() {
	paused = false;
}

void TimeTick::toggle_pause() {
	paused = !paused;
}

bool TimeTick::is_paused() const {
	return paused;
}

void TimeTick::reset() {
	current_tick = 0;
	accumulated_time = 0.0;
	unit_counters.clear();
	
	Array keys = time_units.keys();
	for (int i = 0; i < keys.size(); i++) {
		String unit_name = keys[i];
		Dictionary time_unit = time_units[unit_name];
		time_unit["current_value"] = 0;
		time_units[unit_name] = time_unit;
		unit_counters[unit_name] = 0;
	}
}

void TimeTick::set_time_scale(double scale) {
	time_scale = Math::max(0.0, scale);
}

double TimeTick::get_time_scale() const {
	return time_scale;
}

int TimeTick::get_current_tick() const {
	return current_tick;
}

double TimeTick::get_tick_progress() const {
	if (tick_time <= 0.0) {
		return 0.0;
	}
	return CLAMP(accumulated_time / tick_time, 0.0, 1.0);
}

bool TimeTick::is_initialized() const {
	return initialized;
}

// Private methods

void TimeTick::_on_physics_frame() {
	if (!initialized) {
		return;
	}
	
	double current_time = Time::get_singleton()->get_ticks_msec() / 1000.0;
	double delta = current_time - last_physics_time;
	last_physics_time = current_time;
	_process_tick(delta);
}

void TimeTick::_process_tick(double delta) {
	if (paused) {
		return;
	}
	
	// Apply time scale
	double scaled_delta = delta * time_scale;
	accumulated_time += scaled_delta;
	
	// Process all ticks that should have occurred
	while (accumulated_time >= tick_time) {
		accumulated_time -= tick_time;
		current_tick += 1;
		
		// Increment the "tick" unit
		_increment_unit("tick");
		
		// Emit signal
		emit_signal("tick_updated", current_tick);
	}
}

void TimeTick::_increment_unit(const String &unit_name) {
	// Find all units that track this unit
	Array keys = time_units.keys();
	
	for (int i = 0; i < keys.size(); i++) {
		String child_unit_name = keys[i];
		Dictionary time_unit = time_units[child_unit_name];
		
		String tracked = time_unit["tracked_unit"];
		if (tracked == unit_name) {
			// Get or initialize the counter for THIS CHILD UNIT
			// Counter tracks how much the parent has accumulated (in parent's step units)
			int counter = 0;
			if (unit_counters.has(child_unit_name)) {
				counter = unit_counters[child_unit_name];
			}
			
			// Get the parent's step amount to know how much to add to counter
			int parent_step = 1;
			if (time_units.has(unit_name)) {
				Dictionary parent_unit = time_units[unit_name];
				parent_step = parent_unit["step_amount"];
			}
			
			counter += parent_step;  // Add parent's step, not just 1!
			
			int trigger_count = time_unit["trigger_count"];
			
			// DEBUG
			if (child_unit_name == "minute") {
				UtilityFunctions::print(vformat("[DEBUG] minute counter: %d/%d (parent_step=%d)", 
					counter, trigger_count, parent_step));
			}
			
			// Check if we've accumulated enough to trigger
			if (counter >= trigger_count) {
				// Reset counter (or subtract trigger_count to handle overflow)
				counter -= trigger_count;
				unit_counters[child_unit_name] = counter;
				
				int old_value = time_unit["current_value"];
				int step = time_unit["step_amount"];
				int max_value = time_unit["max_value"];
				int new_value = old_value + step;
				
				// Count how many times we overflow/wrap
				int overflow_count = 0;
				
				// Handle wrapping if max_value is set
				if (max_value > 0) {
					while (new_value >= max_value) {
						new_value -= max_value;
						overflow_count++;
					}
					while (new_value < 0) {
						new_value += max_value;
					}
				}
				
				// Update the value
				time_unit["current_value"] = new_value;
				time_units[child_unit_name] = time_unit;
				
				// Emit change signal
				if (old_value != new_value) {
					emit_signal("time_unit_changed", child_unit_name, new_value, old_value);
				}
				
				// ALWAYS trigger children when this unit increments
				_increment_unit(child_unit_name);
				
				// If we overflowed, trigger additional times for each overflow beyond the first
				if (max_value > 0 && overflow_count > 1) {
					UtilityFunctions::print(vformat("[DEBUG] %s overflowed %d times (additional triggers)", 
						child_unit_name, overflow_count - 1));
					for (int j = 1; j < overflow_count; j++) {
						_increment_unit(child_unit_name);
					}
				}
			} else {
				// Update the counter (not yet triggered)
				unit_counters[child_unit_name] = counter;
			}
		}
	}
}

// Bind methods

void TimeTick::_bind_methods() {
	// Signals
	ADD_SIGNAL(MethodInfo("tick_updated", PropertyInfo(Variant::INT, "current_tick")));
	ADD_SIGNAL(MethodInfo("time_unit_changed", 
		PropertyInfo(Variant::STRING, "unit_name"), 
		PropertyInfo(Variant::INT, "new_value"), 
		PropertyInfo(Variant::INT, "old_value")));
	
	// Methods
	ClassDB::bind_method(D_METHOD("initialize", "tick_duration"), &TimeTick::initialize, DEFVAL(1.0));
	ClassDB::bind_method(D_METHOD("register_time_unit", "unit_name", "tracked_unit", "trigger_count", "step_amount", "max_value", "starting_value"), 
		&TimeTick::register_time_unit, DEFVAL(1), DEFVAL(1), DEFVAL(-1), DEFVAL(0));
	ClassDB::bind_method(D_METHOD("unregister_time_unit", "unit_name"), &TimeTick::unregister_time_unit);
	ClassDB::bind_method(D_METHOD("set_time_unit_step", "unit_name", "step_amount"), &TimeTick::set_time_unit_step);
	ClassDB::bind_method(D_METHOD("get_time_unit", "unit_name"), &TimeTick::get_time_unit);
	ClassDB::bind_method(D_METHOD("set_time_unit", "unit_name", "value"), &TimeTick::set_time_unit);
	ClassDB::bind_method(D_METHOD("set_time_units", "values"), &TimeTick::set_time_units);
	ClassDB::bind_method(D_METHOD("get_time_unit_names"), &TimeTick::get_time_unit_names);
	ClassDB::bind_method(D_METHOD("get_formatted_time", "format_string"), &TimeTick::get_formatted_time);
	ClassDB::bind_method(D_METHOD("get_formatted_time_padded", "units", "separator", "padding"), 
		&TimeTick::get_formatted_time_padded, DEFVAL(":"), DEFVAL(2));
	ClassDB::bind_method(D_METHOD("shutdown"), &TimeTick::shutdown);
	ClassDB::bind_method(D_METHOD("pause"), &TimeTick::pause);
	ClassDB::bind_method(D_METHOD("resume"), &TimeTick::resume);
	ClassDB::bind_method(D_METHOD("toggle_pause"), &TimeTick::toggle_pause);
	ClassDB::bind_method(D_METHOD("is_paused"), &TimeTick::is_paused);
	ClassDB::bind_method(D_METHOD("reset"), &TimeTick::reset);
	ClassDB::bind_method(D_METHOD("set_time_scale", "scale"), &TimeTick::set_time_scale);
	ClassDB::bind_method(D_METHOD("get_time_scale"), &TimeTick::get_time_scale);
	ClassDB::bind_method(D_METHOD("get_current_tick"), &TimeTick::get_current_tick);
	ClassDB::bind_method(D_METHOD("get_tick_progress"), &TimeTick::get_tick_progress);
	ClassDB::bind_method(D_METHOD("is_initialized"), &TimeTick::is_initialized);
}
