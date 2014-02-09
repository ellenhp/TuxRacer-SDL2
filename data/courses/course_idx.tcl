
#
# Procedure to get course name, author, and par time from course.tcl file.
#
proc get_course_info { } {
    if [catch {open course.tcl r} fileId] {
	puts stderr "Couldn't open course.tcl in [pwd]"
	return {}
    }

    set name ""
    set author ""
    set par_time ""

    while {[gets $fileId line] >= 0} {
	regexp {tux_course_name *([^;]*)} $line match name
	regexp {tux_course_author *([^;]*)} $line match author
	regexp {tux_par_time *([^;]*)} $line match par_time

	if { $author != "" && $name != "" && $par_time != "" } {
	    break;
	}
    }

    if { $author == "" } {
	set author "Unknown"
    }

    if { $name == "" } {
	set name "Unknown"
    }

    if { $par_time == "" } {
        set par_time 120.0
    } 

    # Remove quotes around strings, etc.; e.g. "Jasmin Patry" -> Jasmin Patry
    eval "set name $name"
    eval "set author $author"
    eval "set par_time $par_time"

    return [list $name $author $par_time];
}

set cwd [pwd]
tux_goto_data_dir

tux_load_texture noicon textures/noicon.png

tux_load_texture no_preview courses/textures/hud/nopreview.png
tux_bind_texture no_preview no_preview

tux_open_courses [concat \
    { \
    { \
        -course bunny_hill -name "Bunny Hill" \
            -description "Use clever turning to conquer Bunny Hill.  Eat all the herring to earn a higher score!" \
                    -par_time 40.0 \
                    -conditions sunny -no_speed \
    } \
    { \
        -course frozen_river -name "Frozen River" \
            -description "Don't get stuck in the Frozen River!" \
                    -par_time 80.0 \
                    -conditions sunny \
    } \
    { \
        -course twisty_slope -name "Twisty Slope" \
            -description "Tight twists make grabbing herring difficult.  Hard turns will lead you to victory. Only available in full version." \
                    -par_time 40.0  -no_speed \
    } \
    { \
        -course bumpy_ride -name "Bumpy Ride" \
            -description "This hill has a series of ramps to tackle.  Make sure to line yourself up before getting airborne. Only available in full version." \
                    -par_time 40.0 \
                    -conditions sunny -no_speed \
    } \
    { \
        -course penguins_cant_fly -name "Flying Penguins" \
            -description "Go fast, and try to keep a bit of control to catch herrings! Only available in full version." \
                    -par_time 120.0 \
    } \
    { \
        -course slippy_slidey -name "Slippy Slidey" \
            -description "Choose your way to be the best! Only available in full version." \
                    -par_time 120.0 \
                    -conditions sunny \
    } \
    { \
        -course chinese_wall -name "Chinese Wall" \
            -description "Try to stay on the wall to get the most Herring. Only available in full version." \
                    -par_time 120.0 \
                    -conditions sunny \
    } \
    { \
        -course bobsled_ride -name "Bobsled Ride" \
            -description "Just like a real bobsled ride, only more dangerous! Only available in full version." \
                    -par_time 80.0 -conditions sunny \
    } \
    { \
        -course ski_jump -name "Ski Jump" \
            -description "Try to get the longest flying time. Do tricks to increase your score. Make sure to pick up herring too!" \
                    -par_time 40.0 \
                    -conditions evening -no_speed \
    } \
    { \
        -course half_pipe -name "Half Pipe" \
            -description "Do tricks on this course to get a higher score. Make sure to watch the time remaining. Only available in full version." \
                    -par_time 80.0 \
                    -conditions sunny -no_speed \
    } \
    { \
        -course off_piste_skiing -name "Off-Piste Skiing" \
            -description "Free-ride in the mountain! Be careful of trees and use jumps to take short paths. Only available in full version." \
                    -par_time 120.0 \
                    -conditions evening \
    } \
    { \
        -course in_search_of_vodka -name "In Search of Vodka" \
            -description "Tux needs some vodka to warm up is cold belly. Join Tux on the quest for vodka. Pick up herring for dinner along the way! Only available in full version." \
                    -par_time 120.0 \
                    -conditions night \
    } \
    } \
]

#cd courses

tux_load_texture herring_run_icon courses/textures/hud/herringrunicon.png 0
tux_load_texture cup_icon courses/textures/hud/cupicon.png 0

tux_events {
    {
        -name "Tutorials" -icon herring_run_icon -cups {
            {
                -name "Tutorials" -icon cup_icon -races {
                    {
                        -course bunny_hill \
                                -name "Basic tutorial" \
                                -description "Learn here the basics commands needed to play the game." \
                                -herring { 0 0 0 0 } \
                                -time { 37 37 37 37 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions sunny \
                                -windy no -snowing no
                    }
                    {
                       -course frozen_river \
                                -name "Jump tutorial" \
                                -description "Learn here all the things you can do in the air to get better !" \
                                -herring { 0 0 0 0 } \
                                -time { 42 42 42 42 } \
                                -score { 0 0 0 0 } \
                                -mirrored no -conditions sunny \
                                -windy no -snowing no
                    }
                }
             }            
        
			
		}
	}	
}


