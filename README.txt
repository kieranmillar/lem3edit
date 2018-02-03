Lemmings 3 Level Editor (lem3edit GIT)

https://github.com/kieranmillar/lem3edit
================================================================================

On first run of this program, follow the instructions it will provide to point the
program at your L3CD.EXE file. This is part of the files found on the Lemmings 3 CD,
these files you will need to extract onto your harddrive before running this program.

On case-sensitive filesystems, all the data files must have upper case filenames.

Use the left mouse button to select and place objects from the object bar.
Click on objects in the level to select them, and then drag selected objects to move them.

Drag with the right mouse button to scroll the view.

== Keys ========================================================================
s             -- save level
q             -- quit

1             -- switch object bar to background pieces
2             -- switch object bar to terrain pieces
3             -- switch object bar to tool and creature pieces
ctrl + 1      -- toggle visibility of background layer
ctrl + 2      -- toggle visibility of terrain layer
ctrl + 3      -- toggle visibility of tool and creature layer
z             -- scroll object bar left
x             -- scroll object bar right

a             -- select all on visible layers
ctrl          -- select multiple
alt           -- area select
escape        -- select none
delete        -- delete selected objects
ctrl + c      -- copy
ctrl + v      -- paste

,             -- move selected objects behind others on the same layer
.             -- move selected objects infront of others on the same layer
space         -- toggle start camera visibility
p             -- edit level properties

scroll-wheel  -- zoom in and out
ijkl          -- scroll
arrow pad     -- fine-tune object location
shift         -- ... faster

[             -- cycle object id down
]             -- cycle object id up

== Credits =====================================================================

Originally by Carl Reinke https://bitbucket.org/mindless/lemmings-tools
Updating by Kieran Millar
Thanks to Simon from Lemmings Forums for help and CMake support

This program uses:
SDL2 https://www.libsdl.org/index.php
SDL_ttf https://www.libsdl.org/projects/SDL_ttf/
Tiny File Dialogs https://sourceforge.net/projects/tinyfiledialogs/