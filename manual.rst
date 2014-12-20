=============================
Making free animated graphics
=============================

This document explains how to make animation files that the CorsixTH program
can load and use in the game.

In CorsixTH, everything is animated. A swinging door, a walking nurse, or an
operation being performed. Even a desk or a plant is animated (repeatedly
showing the same picture, so the object looks like it is standing still).

In the general case however, an animation has different pictures showing
through time. A single picture displayed at a point in time is called
a *frame*. By showing slightly different pictures with each frame, and
displaying enough frames per second, the illusion of fluent motion is created.

A frame may consist of a single *sprite*, but it may also have many different
sprites. The advantages of having several sprites that together make up the
frame is that you can compose an animation, different parts of the animation
can be added, removed, or changed independently. Also, it enables layering of
the sprites. Sprites are rendered first to last. By adding an operating table
as one of the first elements, it can stay the same no matter what is moving on
top or in front of it. Last but not least, sprites can be re-used between
animations. All of this enables reduction of the number of sprites, which
translates to fewer sprites, and eventually reduced memory usage.


Animation specification file
============================
In an animation specification file, the animations are defined. It looks
like::

    animation "foo" {
        tile_size = 64;
        view = north;

        frame {
            sound = 1;

            element {
                base = "spritefile.png";

                x_offset = -25;
                y_offset = -30;
                alpha = 50;
                hor_flip;
            }

            element {
                ...
            }
        }

        frame {
            ...
        }

        ...
    }

    animation "..." {
        ...
    }

    ...

The file has a sequence of ``animation`` blocks. Each animation has a name
(``"foo"`` in the example), and properties ``tile_size`` and ``view``. The
former is optional, and defaults to 64. At the time of writing this manual,
that is also the only valid value for the tile size. In the future, the
CorsixTH program may be extended with other tile sizes.

The ``view`` property indicates that this ``"foo"`` animation is for the case
where the object being animated has been built in ``north`` direction. Often
several animations exist with the same name, but a different viewing direction
(``north``, ``east``, ``south``, and ``west`` directions exist). For example a
swinging door in a north-south wall versus a swinging door in an east-west
wall. At first, only some of the possible directions will be used by the
CorsixTH program.

Below the properties of the animations come the frames. An animation has at
least one frame, but there can be dozens to hundreds frames, depending on the
length of the animation. All animations with the same name (but different view
direction) must have the same number of frames. (It would be weird if a
treatment would be faster in one direction and slower in another direction.)

Each frame starts with a ``frame`` keyword. Directly below a ``frame`` are its
properties, namely ``sound``. If a non-zero value is used, it denotes that the
sound matching with that number should be played when this frame is displayed.

In a frame, there are one or more ``element`` blocks. Each element defines a
sprite to be shown at a stated position. A simple case is that a sprite has
its own ``.png`` file, as shown here (other cases are explained below).
If a sprite is in its own sprite file, only mentioning the name of the
``.png`` file is sufficient as in ``base = "spritefile.png;``. The next
problem is where to position the sprite in the animation. Each animation has
an origin point (where depends on the animation). The ``x_offset`` and
``y_offset`` define the number of pixels to the right and down, relative to
the origin, to display the sprite. (The origin is often near the bottom of the
animation, these number are usually negative, since ``-25`` to the right means
``25`` to the left, similarly, ``-30`` down means ``30`` up.)

After taking a sprite from its file, it is cropped. The offsets are adjusted
as well, so the final position of the sprite is not changed.

The ``alpha = 50;`` is an effect to make the sprite semi-transparent (``50%``
opaque). You can also use ``75`` for 75% opaqueness, and of course ``100`` for
full opaqueness.
The ``hor_flip`` flips right and left of the sprite, to save you from making a
new sprite. There is also a ``vert_flip`` for flipping the sprite vertically.
The flip settings do not take a value.

Taking a sprite from a sprite sheet
===================================
A second common form of sprites is to have a sprite sheet, a large image
file, containing many sprites, each in its own area of the image. Only
mentioning the file is not sufficient any more in that case, you also have to
define the area to take from the file. That looks like::

    base = "spritesheet.png";
    left = 10;
    top = 110;
    width = 60;
    height = 40;

The name of the sprite sheet file is given. The ``left`` and ``top`` define
the top-left coordinate in the sheet of the sprite where the area that is
wanted, starts. The ``width`` defines the horizontal length, while the
``height`` defines the vertical length.

The given area is taken from the file, and treated like that area is the entire
file. The other steps are just as explained above, the sprite gets offsets, it
is cropped, and effects are applied.

Recolour images
===============
Recolouring is the process of changing the colour of part of the sprite, for
example to have different colours of cloths.

It is currently unclear how much use this will have (conditional displaying
explained below is another way to achieve it too), but the future will tell.

Recolouring can be done with a single sprite file, or with a sprite sheet. The
example here shows the case of a sprite file. In an ``element``, it looks
like::

    base = "spritefile.png";
    recolour = "recolourfile.png";

    layer 4 = 2;

The ``base`` is as before, it defines a 32bpp image file. The ``recolour``
also defines a file, but that must be an 8bpp image file with the same image
dimensions as the file given in ``base``. The palette of the 8bpp image is not
used, only the numeric values are of interest. For each pixel of the sprite
(in case of a sprite sheet, each pixel in the defined area), the numeric index
of the pixel at the same position in the recolour file is consulted to decide
what to do. If its numeric value is 0, no recolouring is applied. If it is any
other value (for example, it is ``1``), the intensity of the 32bpp pixel is
determined instead. That intensity is used by the CorsixTH program as index in
recolour table 1 to decide the actual colour to display.

Note that recolour table 1 will change depending on how to recolour. Non-zero
numeric values in the 8bpp images are therefore like logical layers (a
``cloth`` layer), rather than pointing to a known table.

If the numeric values in the recolour 8bpp file are not correct, ``layer x
= y;`` can be used to change them. The meaning of such a statement is 'if you
encounter numeric value x, treat it as-if it is value y'. The renumbering is
not transitive, for example::

    layer 1 = 2;
    layer 2 = 1;

will swap layers 1 and 2.


Conditional displaying
======================
CorsixTH also has a conditional display feature for an element. Confusingly,
this feature uses LayerClass and LayerId in the program. To avoid confusion
with recolour layers, the animation encoder program names it *conditional
displaying*. If you write the line::

    display_if 3 = 2;

in an element, the element will be displayed if and only if the value of layer
class ``3`` is equal to ``2``.

At the time of writing, there is limited information available on these
display condition values. The documentation says there are thirteen layer
classes (``0`` to ``12``), and layer classes ``0`` and ``1`` are ignored (that
is, the element is always rendered, no matter what value you specify).

As more information becomes available, it may be useful to add names for
classes and ids to the animation encoder, to increase readability.


Loading animation files into CorsixTH
=====================================

Not yet known.


Compiling the animation encoder program
=======================================
In the ``AnimationEncoder`` directory are the source files of the ``encode``
program, that takes an animation specification file, and produces an animation
data file that can be read by CorsixTH.

To build the entire program from its sources, you will need a scanner
generator (*lex* or *flex*), and a parser generator (*yacc* or *bison*). To
compile all code, you need a C++ compiler, for example *g++*. The code uses
``libpng`` for reading the images, so that library must be available to build
against as well. The build process for a typical Linux machine is defined in
the ``mk`` file.

If you don't have a scanner generator or a parser generator, the source code
that they generate is also included in the directory, allowing you to skip
those generation steps.


.. vim: tw=78 spell sw=4 sts=4
