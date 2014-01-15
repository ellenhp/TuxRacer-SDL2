tux_load_texture envmap courses/textures/conditions/envmap.png 0
tux_bind_texture terrain_envmap envmap

tux_particle_colour { 0.85 0.9 1.0 1.0 }

tux_load_texture sky_front courses/textures/conditions/sunnyfront.png 0
tux_load_texture sky_right courses/textures/conditions/sunnyright.png 0
tux_load_texture sky_left courses/textures/conditions/sunnyleft.png 0
tux_load_texture sky_back courses/textures/conditions/sunnyback.png 0
tux_load_texture sky_top courses/textures/conditions/sunnytop.png 0
tux_load_texture sky_bottom courses/textures/conditions/sunnybottom.png 0

tux_bind_texture sky_front sky_front
tux_bind_texture sky_right sky_right
tux_bind_texture sky_left sky_left
tux_bind_texture sky_back sky_back
tux_bind_texture sky_top sky_top
tux_bind_texture sky_bottom sky_bottom
