# Tux Racer initialization script

#
# Save data directory
# 
set tux_data_dir [pwd]

#
# Useful subroutines
#
proc tux_goto_data_dir {} {
    global tux_data_dir
}

proc tux_course_init {} {
    global tux_data_dir
    tux_eval "courses/common/courseinit.tcl"
} 

#
# Read course index
#
tux_eval courses/course_idx.tcl

#
# Set Up Music
#

# Splash & Start screens
# Not used: if { [tux_load_music start_screen music/start1-jt.it] } {
# Not used:     tux_bind_music splash_screen start_screen -1
# Not used:     tux_bind_music start_screen start_screen -1
# Not used: }

# Options screen
if { [tux_load_music options_screen music/options1-jt.it] } {
    tux_bind_music credits_screen options_screen 1
}

# Music played during race
if { [tux_load_music racing music/race1-jt.it] } {
    tux_bind_music intro racing -1
    tux_bind_music racing racing 1
    tux_bind_music paused racing -1
}

# Game Over screen
if { [tux_load_music game_over music/wonrace1-jt.it] } {
    tux_bind_music game_over game_over 1
}


#
# Set Up Music
#

# Splash & Start screens
if { [tux_load_music start_screen music/start1-jt.it] } {
    tux_bind_music splash_screen start_screen -1
    tux_bind_music start_screen start_screen -1
}

# Options screen
if { [tux_load_music options_screen music/options1-jt.it] } {
    tux_bind_music credits_screen options_screen -1
}

# Music played during race
if { [tux_load_music racing music/race1-jt.it] } {
    tux_bind_music intro racing -1
    tux_bind_music racing racing -1
    tux_bind_music paused racing -1
}

# Game Over screen
if { [tux_load_music game_over music/wonrace1-jt.it] } {
    tux_bind_music game_over game_over 1
}


#
# Set Up Sounds
#

# Tree Hit
tux_load_sound tree_hit1 sounds/tux_hit_tree1.wav
tux_bind_sounds hit_tree tree_hit1

# Fish Pickup
if { [tux_load_sound fish_pickup_sound_1 sounds/fish_pickup1.wav]&& 
     [tux_load_sound fish_pickup_sound_2 sounds/fish_pickup2.wav]&& 
     [tux_load_sound fish_pickup_sound_3 sounds/fish_pickup3.wav] } \
{
    tux_bind_sounds item_collect fish_pickup_sound_1 \
	                         fish_pickup_sound_2 \
				 fish_pickup_sound_3
}

# Snow Sliding
if { [tux_load_sound snow_sound sounds/tux_on_snow1.wav] } {
    tux_bind_sounds snow_sound snow_sound
    tux_bind_sounds flying_sound snow_sound
}

# Rock Sliding
if { [tux_load_sound rock_sound sounds/tux_on_rock1.wav] } {
    tux_bind_sounds rock_sound rock_sound
}

# Ice Sliding
if { [tux_load_sound ice_sound sounds/tux_on_ice1.wav] } {
    tux_bind_sounds ice_sound ice_sound
}

# Track marks
# Disabled because not used
tux_load_texture track_head courses/textures/terrain/buttstart.png 1
tux_load_texture track_mark courses/textures/terrain/buttprint.png 1
tux_load_texture track_tail courses/textures/terrain/buttstop.png 1
tux_bind_texture track_head track_head
tux_bind_texture track_mark track_mark
tux_bind_texture track_tail track_tail

# Splash screen
tux_load_texture logo textures/logo.png 0
tux_bind_texture logo logo

# Snow Particle
tux_load_texture snow_particle courses/textures/terrain/snowparticles.png 0
tux_bind_texture snow_particle snow_particle

# Fonts
tux_load_texture trebuchet_outline fonts/trebuchet_outline.png 0
tux_load_font -name trebuchet_outline -file fonts/trebuchet.tfm \
              -texture trebuchet_outline -colour { 1.00 0.89 0.01 1.0 }
tux_bind_font -binding time_label -font trebuchet_outline -size 20 \
			  -colour { 1.00 0.89 0.01 1.0 }
tux_bind_font -binding time_value -font trebuchet_outline -size 30 \
			  -colour { 1.00 0.89 0.01 1.0 }
tux_bind_font -binding time_hundredths -font trebuchet_outline -size 20 \
			  -colour { 1.00 0.89 0.01 1.0 }
tux_bind_font -binding herring_count -font trebuchet_outline -size 30 \
			  -colour { 1.00 0.89 0.01 1.0 }
tux_bind_font -binding speed_digits -font trebuchet_outline -size 35 \
			  -colour { 1.00 0.89 0.01 1.0 }
tux_bind_font -binding speed_units -font trebuchet_outline -size 20 \
			  -colour { 1.00 0.89 0.01 1.0 }
tux_bind_font -binding fps -font trebuchet_outline -size 20 \
			  -colour { 1.00 0.89 0.01 1.0 }

tux_load_texture trebuchet_white fonts/trebuchet_white.png 0
tux_load_font -name trebuchet_white -file fonts/trebuchet.tfm \
              -texture trebuchet_white
tux_bind_font -binding bonus -font trebuchet_outline -size 15 \
			              -colour { 1.00 0.0 0.00 1.0 }
tux_bind_font -binding button_label -font trebuchet_white -size 30
tux_bind_font -binding button_label_hilit -font trebuchet_white -size 30 \
              -colour { 1.00 0.89 0.01 1.0 }
tux_bind_font -binding button_label_disabled -font trebuchet_white -size 30 \
              -colour { 1.0 1.0 1.0 0.5 }

tux_bind_font -binding menu_label -font trebuchet_white -size 30
tux_bind_font -binding course_name_label -font trebuchet_white -size 24 \
              -colour { 1.00 0.89 0.01 1.0 }
tux_bind_font -binding listbox_item -font trebuchet_white -size 22 \
              -colour { 1.00 0.89 0.01 1.0 }
tux_bind_font -binding event_and_cup_label -font trebuchet_white -size 22 
tux_bind_font -binding race_description -font trebuchet_white -size 16
tux_bind_font -binding race_requirements_label -font trebuchet_white -size 13 \
              -colour { 1.00 0.95 0.01 1.0 }
tux_bind_font -binding race_requirements -font trebuchet_white -size 13
tux_bind_font -binding cup_status -font trebuchet_white -size 16 \
              -colour { 1.00 0.89 0.01 1.0 }
tux_bind_font -binding racing_mode_info -font trebuchet_white -size 13 \
			              -colour { 1.00 1.00 1.00 1.0 }
tux_bind_font -binding race_over -font trebuchet_outline -size 40\
			  -colour { 1.00 0.89 0.01 1.0 }
tux_bind_font -binding race_stats -font trebuchet_outline -size 25\
			  -colour { 1.00 0.89 0.01 1.0 }
tux_bind_font -binding race_stats_label -font trebuchet_outline -size 25\
			  -colour { 1.00 0.89 0.01 1.0 }
tux_bind_font -binding race_result_msg -font trebuchet_outline -size 25\
			  -colour { 1.00 0.89 0.01 1.0 }
tux_bind_font -binding paused -font trebuchet_outline -size 40\
			  -colour { 1.00 0.89 0.01 1.0 }

tux_bind_font -binding credits_h1 -font trebuchet_outline -size 30 \
			  -colour { 1.00 0.89 0.01 1.0 }
tux_bind_font -binding credits_h2 -font trebuchet_outline -size 20 \
			  -colour { 1.00 0.89 0.01 1.0 }
tux_bind_font -binding credits_text -font trebuchet_white -size 20
tux_bind_font -binding credits_text_small -font trebuchet_white -size 14

tux_bind_font -binding loading -font trebuchet_outline -size 30 \
			  -colour { 1.00 0.89 0.01 1.0 }
			  
tux_bind_font -binding leaderboard_text -font trebuchet_white -size 20
tux_bind_font -binding race_selection_title -font trebuchet_white -size 40 \
			  -colour { 1.00 0.89 0.01 1.0 }
tux_bind_font -binding race_selection_arrows -font trebuchet_white -size 60 \
			  -colour { 1.00 0.89 0.01 1.0 }

tux_load_texture trebuchet_red fonts/trebuchet_red.png 0
tux_load_font -name trebuchet_red -file fonts/trebuchet.tfm \
              -texture trebuchet_red
tux_bind_font -binding instructions -font trebuchet_red -size 20

# HUD
tux_load_texture herring_icon textures/herringicon.png 0
tux_bind_texture herring_icon herring_icon

tux_load_texture gauge_outline textures/gaugeoutline.png 0
tux_bind_texture gauge_outline gauge_outline

tux_load_texture gauge_energy_mask textures/gaugeenergymask.png 0
tux_bind_texture gauge_energy_mask gauge_energy_mask

tux_load_texture gauge_speed_mask textures/gaugespeedmask.png 0
tux_bind_texture gauge_speed_mask gauge_speed_mask

#HUD Training
tux_load_texture red_circle textures/redCircle.png 0
tux_bind_texture red_circle red_circle

# UI widgets
tux_load_texture listbox_arrows textures/listbox_arrows.png 0
tux_bind_texture listbox_arrows listbox_arrows
tux_load_texture textarea_arrows textures/textarea_arrows.png 0
tux_bind_texture textarea_arrows textarea_arrows

# Racing menu icons
tux_load_texture time_icon textures/timeicon.png 0
tux_bind_texture time_icon time_icon

tux_load_texture mirror_button textures/mirror_button.png 0
tux_bind_texture mirror_button mirror_button

tux_load_texture conditions_button textures/conditions_button.png 0
tux_bind_texture conditions_button conditions_button

tux_load_texture snow_button textures/snow_button.png 0
tux_bind_texture snow_button snow_button

tux_load_texture wind_button textures/wind_button.png 0
tux_bind_texture wind_button wind_button


# Menu decorations
tux_load_texture menu_bottom_left textures/menu_bottom_left.png 0
tux_bind_texture menu_bottom_left menu_bottom_left

tux_load_texture menu_bottom_right textures/menu_bottom_right.png 0
tux_bind_texture menu_bottom_right menu_bottom_right

tux_load_texture menu_top_left textures/menu_top_left.png 0
tux_bind_texture menu_top_left menu_top_left

tux_load_texture menu_top_right textures/menu_top_right.png 0
tux_bind_texture menu_top_right menu_top_right

tux_load_texture menu_title textures/menu_title.png 0
tux_bind_texture menu_title menu_title

# Tux life icon
tux_load_texture tux_life textures/tuxlife.png 0
tux_bind_texture tux_life tux_life

# Mouse cursor
tux_load_texture mouse_cursor textures/mouse_cursor.png 0
tux_bind_texture mouse_cursor mouse_cursor
