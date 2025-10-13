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
	if (tick_duration <= 0.0) {
		UtilityFunctions::push_warning("TimeTick: Tick duration must be greater than 0.0, clamping to 0.001");
		tick_duration = 0.001;
	}
	tick_time = CLAMP(tick_duration, 0.001, 600.0);
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

void TimeTick::register_time_unit(const String &unit_name, const String &tracked_unit, int trigger_count, int max_value, int min_value) {
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
	time_unit["current_value"] = min_value;  // Starting value defaults to min_value
	time_unit["tracked_unit"] = tracked_unit;
	time_unit["trigger_count"] = trigger_count;
	time_unit["step_amount"] = 1;  // Default step_amount is 1
	time_unit["max_value"] = max_value;
	time_unit["min_value"] = min_value;
	
	time_units[unit_name] = time_unit;
	
	// Initialize counter for this unit
	if (!unit_counters.has(unit_name)) {
		unit_counters[unit_name] = 0;
	}
}

void TimeTick::register_complex_time_unit(const String &unit_name, const Dictionary &tracked_units, int max_value, int min_value) {
	if (unit_name.is_empty()) {
		UtilityFunctions::push_error("TimeTick: Unit name cannot be empty");
		return;
	}
	
	if (tracked_units.is_empty()) {
		UtilityFunctions::push_error("TimeTick: Complex time unit must track at least one unit");
		return;
	}
	
	// Validate that all tracked units exist
	Array keys = tracked_units.keys();
	for (int i = 0; i < keys.size(); i++) {
		String tracked_unit = keys[i];
		if (!time_units.has(tracked_unit) && tracked_unit != "tick") {
			UtilityFunctions::push_warning(vformat("TimeTick: Tracked unit '%s' not yet registered, make sure to register it first", tracked_unit));
		}
	}
	
	// Create a Dictionary to store complex TimeUnit data
	Dictionary time_unit;
	time_unit["name"] = unit_name;
	time_unit["current_value"] = min_value;  // Starting value defaults to min_value
	time_unit["is_complex"] = true;  // Mark this as a complex unit
	time_unit["tracked_units"] = tracked_units;  // Dictionary of {unit_name: trigger_value}
	time_unit["step_amount"] = 1;  // Default step_amount is 1
	time_unit["max_value"] = max_value;
	time_unit["min_value"] = min_value;
	
	time_units[unit_name] = time_unit;
	
	// No counter needed for complex units - we check the actual values directly
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

int TimeTick::get_time_unit_step(const String &unit_name) const {
	if (time_units.has(unit_name)) {
		Dictionary time_unit = time_units[unit_name];
		if (time_unit.has("step_amount")) {
			return time_unit["step_amount"];
		}
	}
	return 0;
}

void TimeTick::set_time_unit_trigger_count(const String &unit_name, int trigger_count) {
	if (trigger_count <= 0) {
		UtilityFunctions::push_error("TimeTick: Trigger count must be positive");
		return;
	}
	
	if (time_units.has(unit_name)) {
		Dictionary time_unit = time_units[unit_name];
		
		// Check if this is a complex unit
		if (time_unit.has("is_complex") && time_unit["is_complex"]) {
			UtilityFunctions::push_error(vformat("TimeTick: Cannot set trigger_count for complex time unit '%s'. Use tracked_units dictionary instead.", unit_name));
			return;
		}
		
		time_unit["trigger_count"] = trigger_count;
		time_units[unit_name] = time_unit;
	} else {
		UtilityFunctions::push_error(vformat("TimeTick: Time unit '%s' not found", unit_name));
	}
}

int TimeTick::get_time_unit_trigger_count(const String &unit_name) const {
	if (time_units.has(unit_name)) {
		Dictionary time_unit = time_units[unit_name];
		
		// Check if this is a complex unit
		if (time_unit.has("is_complex") && time_unit["is_complex"]) {
			UtilityFunctions::push_warning(vformat("TimeTick: Complex time unit '%s' doesn't have a single trigger_count. Use get_time_unit_tracked_units() instead.", unit_name));
			return -1;
		}
		
		if (time_unit.has("trigger_count")) {
			return time_unit["trigger_count"];
		}
	}
	return 0;
}

void TimeTick::set_time_unit_starting_value(const String &unit_name, int starting_value) {
	if (time_units.has(unit_name)) {
		Dictionary time_unit = time_units[unit_name];
		time_unit["min_value"] = starting_value;
		time_units[unit_name] = time_unit;
	} else {
		UtilityFunctions::push_error(vformat("TimeTick: Time unit '%s' not found", unit_name));
	}
}

int TimeTick::get_time_unit_starting_value(const String &unit_name) const {
	if (time_units.has(unit_name)) {
		Dictionary time_unit = time_units[unit_name];
		if (time_unit.has("min_value")) {
			return time_unit["min_value"];
		}
	}
	return 0;
}

Dictionary TimeTick::get_time_unit_data(const String &unit_name) const {
	if (time_units.has(unit_name)) {
		return time_units[unit_name];
	}
	return Dictionary();
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
		
		// Reset this unit's counter to 0
		// Time will progress naturally from the new value
		unit_counters[unit_name] = 0;
		
		// Emit signal (only if value actually changed)
		if (old_value != value) {
			emit_signal("time_unit_changed", unit_name, value, old_value);
		}
	} else {
		UtilityFunctions::push_error(vformat("TimeTick: Time unit '%s' not found", unit_name));
	}
}

void TimeTick::set_time_units(const Dictionary &values) {
	// First, set all the values
	Array keys = values.keys();
	for (int i = 0; i < keys.size(); i++) {
		String unit_name = keys[i];
		int value = values[unit_name];
		if (time_units.has(unit_name)) {
			Dictionary time_unit = time_units[unit_name];
			time_unit["current_value"] = value;
			time_units[unit_name] = time_unit;
		}
	}
	
	// Then recalculate all counters based on what each unit tracks
	Array all_keys = time_units.keys();
	for (int i = 0; i < all_keys.size(); i++) {
		String unit_name = all_keys[i];
		Dictionary time_unit = time_units[unit_name];
		
		// Only update counters for simple (non-complex) units
		if (time_unit.has("tracked_unit") && !time_unit.has("is_complex")) {
			String tracked = time_unit["tracked_unit"];
			
			// Set counter based on the tracked unit's current value
			if (tracked == "tick") {
				// Track the global tick counter
				unit_counters[unit_name] = current_tick;
			} else if (time_units.has(tracked)) {
				Dictionary tracked_unit = time_units[tracked];
				int tracked_value = tracked_unit["current_value"];
				int tracked_step = tracked_unit["step_amount"];
				// Counter = tracked_value * tracked_step
				unit_counters[unit_name] = tracked_value * tracked_step;
			} else {
				// Tracked unit doesn't exist, reset to 0
				unit_counters[unit_name] = 0;
			}
		} else {
			// Complex units or units without tracking, reset to 0
			unit_counters[unit_name] = 0;
		}
	}
	
	// Finally, emit signals for changed values
	for (int i = 0; i < keys.size(); i++) {
		String unit_name = keys[i];
		int value = values[unit_name];
		emit_signal("time_unit_changed", unit_name, value, value);
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
	time_scale = CLAMP(scale, -1000.0, 1000.0);
}

double TimeTick::get_time_scale() const {
	return time_scale;
}

void TimeTick::set_tick_duration(double duration) {
	if (duration <= 0.0) {
		UtilityFunctions::push_warning("TimeTick: Tick duration must be greater than 0.0, clamping to 0.001");
		duration = 0.001;
	}
	tick_time = CLAMP(duration, 0.001, 600.0);
}

double TimeTick::get_tick_duration() const {
	return tick_time;
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
	
	// Handle forward time (positive time_scale)
	if (time_scale >= 0.0) {
		// Process all ticks that should have occurred
		while (accumulated_time >= tick_time) {
			accumulated_time -= tick_time;
			
			// Check for overflow - reset to 0 if we exceed INT_MAX
			if (current_tick >= INT_MAX) {
				current_tick = 0;
				UtilityFunctions::push_warning("TimeTick: Tick count reached maximum value, resetting to 0");
			} else {
				current_tick += 1;
			}
			
			// Increment the "tick" unit
			_increment_unit("tick");
			
			// Emit signal
			emit_signal("tick_updated", current_tick);
		}
	} else {
		// Handle backward time (negative time_scale)
		// accumulated_time will be negative, so we check if it's <= -tick_time
		while (accumulated_time <= -tick_time) {
			accumulated_time += tick_time;
			
			// Check for underflow - reset to 0 if we go below 0
			if (current_tick <= 0) {
				current_tick = 0;
				UtilityFunctions::push_warning("TimeTick: Tick count reached minimum value (0), cannot decrement further");
				// Stop decrementing to prevent going negative
				accumulated_time = 0.0;
				break;
			} else {
				current_tick -= 1;
			}
			
			// Decrement the "tick" unit
			_decrement_unit("tick");
			
			// Emit signal
			emit_signal("tick_updated", current_tick);
		}
	}
}

void TimeTick::_increment_unit(const String &unit_name) {
	// Find all units that track this unit
	Array keys = time_units.keys();
	
	for (int i = 0; i < keys.size(); i++) {
		String child_unit_name = keys[i];
		Dictionary time_unit = time_units[child_unit_name];
		
		// Check if this is a complex unit
		bool is_complex = time_unit.has("is_complex") && (bool)time_unit["is_complex"];
		
		if (is_complex) {
			// Complex unit: check if ALL tracked units have reached or exceeded their trigger values
			Dictionary tracked_units = time_unit["tracked_units"];
			bool should_check = tracked_units.has(unit_name);  // Only check if this unit being incremented is tracked
			
			if (should_check) {
				// Check if ALL tracked units are at or past their required values
				bool all_conditions_met = true;
				bool should_reset = false;  // Only reset when OVERALL time has wrapped
				Array tracked_keys = tracked_units.keys();
				
				for (int j = 0; j < tracked_keys.size(); j++) {
					String tracked_unit = tracked_keys[j];
					int required_value = tracked_units[tracked_unit];
					int current_value = 0;
					
					// Special handling for "tick" unit
					if (tracked_unit == "tick") {
						current_value = current_tick;
					} else {
						current_value = get_time_unit(tracked_unit);
					}
					
					// Check if current value is less than required (hasn't reached it yet)
					if (current_value < required_value) {
						all_conditions_met = false;
						// Only set should_reset if this is a "higher order" unit (not second/minute which wrap frequently)
						// For example, if hour < required_hour, then we've truly wrapped around the day
						if (tracked_unit == "hour" || tracked_unit == "day" || tracked_unit == "month" || tracked_unit == "year") {
							should_reset = true;
						}
						break;
					}
				}
				
				// Get the triggered state
				String state_key = child_unit_name + "_triggered";
				bool was_triggered = false;
				if (time_unit.has(state_key)) {
					was_triggered = (bool)time_unit[state_key];
				}
				
				// If all conditions are NOW met and we haven't triggered yet
				if (all_conditions_met && !was_triggered) {
					int old_value = time_unit["current_value"];
					int step = time_unit["step_amount"];
					int max_value = time_unit["max_value"];
					int min_value = time_unit.has("min_value") ? (int)time_unit["min_value"] : 0;
					int new_value = old_value + step;
					
					// Handle wrapping if max_value is set
					if (max_value > 0) {
						// Range represents the number of possible values [min_value, max_value)
						// max_value is EXCLUSIVE - wraps when reaching max_value
						int range = max_value - min_value;
						while (new_value >= max_value) {
							new_value -= range;
						}
						while (new_value < min_value) {
							new_value += range;
						}
					}
					
					// Update the value and mark as triggered
					time_unit["current_value"] = new_value;
					time_unit[state_key] = true;
					time_units[child_unit_name] = time_unit;
					
					// Emit change signal
					if (old_value != new_value) {
						emit_signal("time_unit_changed", child_unit_name, new_value, old_value);
					}
					
					// Trigger this complex unit's children
					_increment_unit(child_unit_name);
				} else if (should_reset && was_triggered) {
					// A high-order unit fell below threshold - reset the trigger
					// This happens when time wraps (e.g., hour goes from 23 to 0)
					time_unit[state_key] = false;
					time_units[child_unit_name] = time_unit;
				}
			}
		} else {
			// Simple unit: original logic
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
			
			// Check if we've accumulated enough to trigger
			if (counter >= trigger_count) {
				// Reset counter (or subtract trigger_count to handle overflow)
				counter -= trigger_count;
				unit_counters[child_unit_name] = counter;
				
				int old_value = time_unit["current_value"];
				int step = time_unit["step_amount"];
				int max_value = time_unit["max_value"];
				int min_value = time_unit.has("min_value") ? (int)time_unit["min_value"] : 0;
				int new_value = old_value + step;
				
				// Check for overflow before wrapping logic
				if (step > 0 && old_value > INT_MAX - step) {
					// Would overflow, reset to min_value
					new_value = min_value;
					UtilityFunctions::push_warning(vformat("TimeTick: Time unit '%s' would overflow, resetting to %d", child_unit_name, min_value));
				} else if (step < 0 && old_value < INT_MIN - step) {
					// Would underflow, reset to min_value
					new_value = min_value;
					UtilityFunctions::push_warning(vformat("TimeTick: Time unit '%s' would underflow, resetting to %d", child_unit_name, min_value));
				}
				
				// Handle wrapping if max_value is set
				if (max_value > 0) {
					// Range represents the number of possible values [min_value, max_value)
					// max_value is EXCLUSIVE - wraps when reaching max_value
					int range = max_value - min_value;
					bool did_wrap = false;
					while (new_value >= max_value) {
						new_value -= range;
						did_wrap = true;
					}
					while (new_value < min_value) {
						new_value += range;
						did_wrap = true;
					}
					
					// If wrapping occurred, trigger children FIRST before updating this unit
					// This ensures that when this unit's signal emits, children have already updated
					if (did_wrap) {
						// Temporarily set the wrapped value so children can see it
						int temp_old_value = time_unit["current_value"];
						time_unit["current_value"] = new_value;
						time_units[child_unit_name] = time_unit;
						
						// Trigger children - they'll increment based on seeing the wrap
						_increment_unit(child_unit_name);
						
						// Now emit the signal for THIS unit's change
						emit_signal("time_unit_changed", child_unit_name, new_value, old_value);
						
						// Skip the normal update below since we already did it
						continue;
					}
				}
				
				// Update the value first
				time_unit["current_value"] = new_value;
				time_units[child_unit_name] = time_unit;
				
				// Emit change signal
				if (old_value != new_value) {
					emit_signal("time_unit_changed", child_unit_name, new_value, old_value);
				}
				
				// Trigger children after value update
				// This includes both wrapping units and event timers
				_increment_unit(child_unit_name);
				
				// If we overflowed multiple times, trigger additional times
				int overflow_count = 0;
				if (max_value > 0) {
					int test_value = old_value + step;
					while (test_value >= max_value) {
						test_value -= max_value;
						overflow_count++;
					}
					
					for (int j = 1; j < overflow_count; j++) {
						_increment_unit(child_unit_name);
					}
				}
			} else {
				// Update the counter (not yet triggered)
				unit_counters[child_unit_name] = counter;
			}
			}  // End if (tracked == unit_name)
		}  // End else (simple unit)
	}  // End for loop
}

void TimeTick::_decrement_unit(const String &unit_name) {
	// Find all units that track this unit and decrement them
	Array keys = time_units.keys();
	
	for (int i = 0; i < keys.size(); i++) {
		String child_unit_name = keys[i];
		Dictionary time_unit = time_units[child_unit_name];
		
		// Check if this is a complex unit
		bool is_complex = time_unit.has("is_complex") && (bool)time_unit["is_complex"];
		
		if (is_complex) {
			// Complex units don't work well with reverse time, skip for now
			// TODO: Implement reverse logic for complex units if needed
			continue;
		} else {
			// Simple unit: decrement logic
			String tracked = time_unit["tracked_unit"];
			if (tracked == unit_name) {
				// Get or initialize the counter for THIS CHILD UNIT
				int counter = 0;
				if (unit_counters.has(child_unit_name)) {
					counter = unit_counters[child_unit_name];
				}
				
				// Get the parent's step amount to know how much to subtract from counter
				int parent_step = 1;
				if (time_units.has(unit_name)) {
					Dictionary parent_unit = time_units[unit_name];
					parent_step = parent_unit["step_amount"];
				}
				
				counter -= parent_step;  // Subtract parent's step for reverse
				
				int trigger_count = time_unit["trigger_count"];
				
				// Check if we've decremented enough to trigger a reverse
				if (counter < 0) {
					// Wrap counter back
					counter += trigger_count;
					unit_counters[child_unit_name] = counter;
					
					int old_value = time_unit["current_value"];
					int step = time_unit["step_amount"];
					int max_value = time_unit["max_value"];
					int min_value = time_unit.has("min_value") ? (int)time_unit["min_value"] : 0;
					int new_value = old_value - step;  // Subtract instead of add
					
					// Handle wrapping if max_value is set (reverse direction)
					if (max_value > 0) {
						// max_value is EXCLUSIVE - wraps when reaching max_value
						int range = max_value - min_value;
						while (new_value < min_value) {
							new_value += range;
						}
						while (new_value >= max_value) {
							new_value -= range;
						}
					}
					
					// Prevent going below absolute minimum for non-wrapping units
					if (max_value <= 0 && new_value < 0) {
						new_value = 0;
					}
					
					// Update the value first
					time_unit["current_value"] = new_value;
					time_units[child_unit_name] = time_unit;
					
					// Emit change signal
					if (old_value != new_value) {
						emit_signal("time_unit_changed", child_unit_name, new_value, old_value);
					}
					
					// Trigger children after value update
					_decrement_unit(child_unit_name);
				} else {
					// Update the counter (not yet triggered)
					unit_counters[child_unit_name] = counter;
				}
			}  // End if (tracked == unit_name)
		}  // End else (simple unit)
	}  // End for loop
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
	ClassDB::bind_method(D_METHOD("register_time_unit", "unit_name", "tracked_unit", "trigger_count", "max_value", "min_value"), 
		&TimeTick::register_time_unit, DEFVAL(1), DEFVAL(-1), DEFVAL(0));
	ClassDB::bind_method(D_METHOD("register_complex_time_unit", "unit_name", "tracked_units", "max_value", "min_value"),
		&TimeTick::register_complex_time_unit, DEFVAL(-1), DEFVAL(0));
	ClassDB::bind_method(D_METHOD("unregister_time_unit", "unit_name"), &TimeTick::unregister_time_unit);
	ClassDB::bind_method(D_METHOD("set_time_unit_step", "unit_name", "step_amount"), &TimeTick::set_time_unit_step);
	ClassDB::bind_method(D_METHOD("get_time_unit_step", "unit_name"), &TimeTick::get_time_unit_step);
	ClassDB::bind_method(D_METHOD("set_time_unit_trigger_count", "unit_name", "trigger_count"), &TimeTick::set_time_unit_trigger_count);
	ClassDB::bind_method(D_METHOD("get_time_unit_trigger_count", "unit_name"), &TimeTick::get_time_unit_trigger_count);
	ClassDB::bind_method(D_METHOD("set_time_unit_starting_value", "unit_name", "starting_value"), &TimeTick::set_time_unit_starting_value);
	ClassDB::bind_method(D_METHOD("get_time_unit_starting_value", "unit_name"), &TimeTick::get_time_unit_starting_value);
	ClassDB::bind_method(D_METHOD("get_time_unit_data", "unit_name"), &TimeTick::get_time_unit_data);
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
	ClassDB::bind_method(D_METHOD("set_tick_duration", "duration"), &TimeTick::set_tick_duration);
	ClassDB::bind_method(D_METHOD("get_tick_duration"), &TimeTick::get_tick_duration);
	ClassDB::bind_method(D_METHOD("get_current_tick"), &TimeTick::get_current_tick);
	ClassDB::bind_method(D_METHOD("get_tick_progress"), &TimeTick::get_tick_progress);
	ClassDB::bind_method(D_METHOD("is_initialized"), &TimeTick::is_initialized);
}
