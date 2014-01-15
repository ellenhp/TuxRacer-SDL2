#
# Environmental sphere map
    
tux_load_texture alpine1-sphere courses/textures/conditions/nightenv.png 0
tux_bind_texture terrain_envmap alpine1-sphere

tux_particle_colour { 0.39 0.51 0.88 1.0 }

tux_load_texture alpine1-front courses/textures/conditions/nightfront.png 0
tux_load_texture alpine1-right courses/textures/conditions/nightright.png 0
tux_load_texture alpine1-left courses/textures/conditions/nightleft.png 0
tux_load_texture alpine1-back courses/textures/conditions/nightback.png 0
tux_load_texture alpine1-top courses/textures/conditions/nighttop.png 0
tux_load_texture alpine1-bottom courses/textures/conditions/nightbottom.png 0

tux_bind_texture sky_front alpine1-front
tux_bind_texture sky_right alpine1-right
tux_bind_texture sky_left alpine1-left
tux_bind_texture sky_back alpine1-back
tux_bind_texture sky_top alpine1-top
tux_bind_texture sky_bottom alpine1-bottom