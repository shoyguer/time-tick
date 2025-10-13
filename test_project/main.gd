extends Node2D
## TimeTick GDExtension - Example Selector
##
## This is the main entry point for TimeTick examples.
## Each example demonstrates different use cases of the TimeTick system.
##
## To run an example:
## 1. Open the corresponding example script (example_1_standard_time.gd, etc.)
## 2. Attach it to a Node in your scene
## 3. Run the scene
##
## Available Examples:
## - example_1_standard_time.gd      : Standard time system (seconds/minutes/hours/days)
## - example_2_custom_calendar.gd    : Fantasy calendar with custom time units
## - example_3_countdown_timer.gd    : Countdown timer using negative steps
## - example_4_specific_time.gd      : Starting at a specific time (save/load)
## - example_5_control_functions.gd  : Time control (pause/speed/reset)


func _ready() -> void:
	print("\n" + "=".repeat(60))
	print("TimeTick GDExtension - Example Collection")
	print("=".repeat(60))
	print("\nAvailable Examples:")
	print("  1. Standard Time System")
	print("     - Basic time progression: tick → second → minute → hour → day")
	print("     - File: example_1_standard_time.gd")
	print("")
	print("  2. Custom Fantasy Calendar")
	print("     - Custom time hierarchy for fictional worlds")
	print("     - File: example_2_custom_calendar.gd")
	print("")
	print("  3. Countdown Timer")
	print("     - Countdown using negative step values")
	print("     - File: example_3_countdown_timer.gd")
	print("")
	print("  4. Start at Specific Time")
	print("     - Set initial time (useful for save/load systems)")
	print("     - File: example_4_specific_time.gd")
	print("")
	print("  5. Control Functions")
	print("     - Time manipulation (pause, speed up, slow down, reset)")
	print("     - File: example_5_control_functions.gd")
	print("")
	print("=".repeat(60))
	print("To run an example:")
	print("  1. Attach the example script to a Node in your scene")
	print("  2. Run the scene")
	print("=".repeat(60) + "\n")
