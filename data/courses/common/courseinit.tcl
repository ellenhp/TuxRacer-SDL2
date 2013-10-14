#Course initialization script
#sets basic course paramaters before the lighting is set.

tux_goto_data_dir

tux_eval courses/common/tree_polyhedron.tcl

tux_load_texture fish courses/textures/items/herring_standard.png 0
tux_item_spec -name herring -diameter 1.0 -height 1.0 \
      -texture fish -colour {28 185 204} -above_ground 0.2

tux_load_texture shrub courses/textures/models/shrub.png 0
tux_tree_props -name tree3 -diameter 1.4 -height 1.0 \
      -texture shrub -colour {0 255 48} -polyhedron $tree_poly \
      -size_varies 0.5 

tux_load_texture tree courses/textures/models/tree.png 0
tux_tree_props -name tree1 -diameter 1.4 -height 2.5 \
      -texture tree -colour {255 255 255} -polyhedron $tree_poly \
      -size_varies 0.5 

tux_load_texture tree_barren courses/textures/models/tree_barren.png 0
tux_tree_props -name tree2 -diameter 1.4 -height 2.5 \
      -texture tree_barren -colour {255 96 0} -polyhedron $tree_poly \
      -size_varies 0.5 

tux_load_texture flag1 courses/textures/items/flag.png 0
tux_item_spec -name flag -diameter 1.0 -height 1.0 \
      -texture flag1 -colour {194 40 40} -nocollision
      
tux_load_texture finish courses/textures/items/finish.png 0
tux_item_spec -name finish -diameter 9.0 -height 6.0 \
		-texture finish -colour {255 255 0} -nocollision \
                -normal {0 0 1}

tux_load_texture start courses/textures/items/start.png 0
tux_item_spec -name start -diameter 9.0 -height 6.0 \
		-texture start -colour {128 128 0} -nocollision \
                -normal {0 0 1}

tux_item_spec -name float -nocollision -colour {255 128 255} -reset_point


tux_trees "$::course_dir/trees.rgb"


tux_ice_tex courses/textures/terrain/ice.png
tux_rock_tex courses/textures/terrain/rock.png
tux_snow_tex courses/textures/terrain/snow.png

tux_friction 0.22 0.9 0.35

#
# Introductory animation keyframe data
#
tux_eval courses/common/tux_walk.tcl

#
# Lighting
#
set conditions [tux_get_race_conditions]
if { $conditions == "sunny" } {
    tux_eval courses/common/sunny_light.tcl
} elseif { $conditions == "night" } {
    tux_eval courses/common/night_light.tcl
} elseif { $conditions == "evening" } {
    tux_eval courses/common/evening_light.tcl
} 

