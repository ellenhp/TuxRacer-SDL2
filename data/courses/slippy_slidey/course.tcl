#
# Course configuration
#
tux_course_name "Slippy-Slidey"
tux_course_author "Modified from 'Bronzeset' by S.ET."
tux_course_dim 100 1700 90 1700 ;# width, length of course in m
tux_start_pt 50 100           ;# start position, measured from left rear corner
tux_angle 35                   ;# angle of course
tux_elev_scale 20             ;# amount by which to scale elevation data
tux_elev $::course_dir/elev.png              ;# bitmap specifying course elevations
tux_terrain $::course_dir/terrain.png        ;# bitmap specifying terrains type

tux_course_init
