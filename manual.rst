====================
Making free graphics
====================
If the explanation in the ``README.txt`` file is a bit too short, below is a
longer discussion.

`Pixels, images, graphics`_
    It starts with a short discussion of the various topics and terms in this
    part of the world.

`Making new free graphics files`_
    What to actually do to make sprites for the new free graphics, and to load
    them into CorsixTH.

`Compiling`_
    The conversion from images to a free graphics file for CorsixTH is done by
    the ``encode`` program, that you may want to compile yourself.


Pixels, images, graphics
========================
Before discussing how to make new free graphics, first some introductory
explanation of the world of pixels, images, and graphics, to understand what
is meant with each term, and how they relate to each other.

Pixels
~~~~~~
The first item to discuss are properties of a pixel in the graphics. In this
document, there are only two interesting properties, namely colour and
opacity.

The *colour* of a pixel is always expressed as a combination of the amounts of
red, green, and blue. This is commonly known as *RGB* colours. Each amount can
vary between ``0`` and ``255``, giving you more colours than the human eye can
distinguish.

The *opacity* is how much of the background is suppressed. If you imagine two
layers of sprites on top of each other, an opacity of ``0`` of the top layer
means you can completely see the bottom layer (the top layer is fully
transparent). At the other end of the spectrum, an opacity of ``255`` means
you see nothing of the bottom layer through the top layer (the top layer is
fully opaque, or non-transparent).

Sprites and graphics
~~~~~~~~~~~~~~~~~~~~
A pixel is very small. To display for example a letter, you already need a
hundred or more pixels. As a result, everybody only speaks about large
collections of pixels. Such a collection of related pixels is called a
*sprite*. There are programs to create sprites, and display them.

In games, hundreds to thousands of sprites are used. The total collection of
sprites needed for the game is called *graphics*. In CorsixTH, there are two
such sets, the original graphics that came on the Theme Hospital CD, and the
free graphics.

Images
~~~~~~
A sprite also needs to be stored. This is done in an *image*. There are many
different kinds of images, but here only ``.png`` formats are used, in
particular

the **8bpp paletted image format**
    This format is very old, from a time where computers and displays were not
    so powerful and disk space was expensive. There was no room for storing
    the RGB colour of each pixel separately. Instead, a palette was used. A
    *palette* is a table with 256 entries containing RGB colours. Each pixel
    has an index into that table, pointing to its colour.
    As a result, there can be at most 256 different colours in an 8bpp
    paletted image.
    Also for reduction in storage size, all pixels have an opacity of 255.

the **32bpp RGBA image format**
    In this format, each pixel has its own colour and opacity. *RGBA* states
    the order of storage of the pixel properties, with the RGB part being its
    colour, and the A (named 'Alpha') being its opacity.

Recolouring
~~~~~~~~~~~
Many games use a technique called *recolouring* to make the graphics more
diverse. The basic idea is to use the same sprite, but to change the colours
in a systematic way to create a different look. In CorsixTH, this can be seen
in the clothes of the patients. There are only a few different styles of
clothing, but some styles are given a different colour to make it more
diverse.

With 8bpp paletted images, recolouring is easy. The colour of a pixel is
retrieved from the palette, thus recolouring can be done by replacing the
palette of the image with a different one.
CorsixTH keeps track of the palette to use for each object in the game.

With 32bpp RGBA images, there is no palette, so the above trick fails.
Instead, it works with *types of recolouring*. For example, you can have 'cloth
recolouring' or 'equipment recolouring'. For each pixel in the image, it must
be decided whether it should be recoloured, and if so, to what type.
The easiest way to store this information is by using another image, in this
case an 8bpp paletted image. The colour index of this image is however not
pointing to a colour but to a recolour type (where index value ``0`` means 'do
not recolour').

For each type of recolouring, several palettes should be made. For example,
there could be a 'red cloth recolouring' palette, a 'blue cloth recolouring'
palette, and a 'yellow cloth recolouring' palette.

Recolouring works by taking the RGB colour of recoloured pixels, and
translating it to an intensity (maximum value of the red, green, and blue
amounts). The intensity is used as index in the palette for the type of
recolouring of that pixel.


CorsixTH graphics
~~~~~~~~~~~~~~~~~
Images aim to store a single sprites as compactly as possible. In games, there
are hundreds to thousands of sprites. Keeping each one as a separate image
would be cumbersome (you would have a thousand files). In addition, games do
not care about compactness of storage, they need fast rendering of sprites,
since hundreds of sprites are drawn a dozen or more times each second. For
this reason, the CorsixTH game has its own way of storing sprites.

The sprites are split between different files, one file for each large window,
and one for the hospital view. Each file has lots of sprites. A sprite in a
file has a simple sequential number. The first sprite in a file has number
``1``, the second sprite has number ``2``, and so on.

Original graphics
.................
The *original graphics* (from the Theme Hospital CD) are stored in a format
based on the 8bpp paletted format, but with less colours (``0`` to ``63`` for
each of the red, green and blue colours), and aimed at storing many sprites
together. Unlike the 8bpp paletted image format, the CorsixTH format does have
opacity, albeit only for a few values. The program also does recolouring of
the sprites.

Free graphics
.............
The other set of sprites for CorsixTH is called *free graphics* (until someone
invents a better name). Free graphics will replace all original sprites one
day.

The free graphics are based on the RGBA format, making a lot more colour and
opacity options available. The free graphics the same file structure and
sprite numbering of the original graphics, making it easy to relate sprites
from different graphics with each other.


Making new free graphics files
==============================
With all the preliminaries out of the way, the time has come to discuss
creating of new sprites for the free graphics.

Making sprites
~~~~~~~~~~~~~~
The first thing to do is to make one or more new sprites for the game items
that you want to replace with your graphics. Each sprite should have the same
size as the size of the original sprite, that is, have the same width and
height in pixels.

Since free graphics uses the RGBA format, sprites must be provided as ``.png``
file in the 32bpp RGBA image format. If the sprite is recoloured, you also
need to supply an 8bpp paletted file containing the recolour type information.
The paletted file must have the same width and height as the RGBA file, and
the sprite pixels must be at the same positions in both files.

Index ``0`` in the paletted image is used for denoting the pixel should not be
recoloured, and index ``255`` is used internally for displaying the original
graphics.
CorsixTH has a table with recolour types, and their index value. (At the time
of writing, this table does not exist, as the free graphics currently does not
have any recoloured sprites.)

The ``encode`` conversion program that takes your images and creates a free
graphics file, can read several sprites from the same image, thus you can make
a sprite sheet, which may be useful.
It is useful to put the top-left corner of each sprite at an easy to remember
position in the image, for example x and y positions at a multiple of 100.

Making a new file
~~~~~~~~~~~~~~~~~
Once you have created one or more new sprites, they have to be put into a new
graphics file to be read by CorsixTH. The ``encode`` program performs this
function. Here it is assumed that you have this program. If that is not the
case, you may want to create it first. How to do that is explained below, in
`Compiling`_.

The ``encode`` program needs to know what sprites you have. For this purpose
you have to make an input file that contains the details for all new sprites
you wish to encode into a file. For each non-recoloured sprite, you should
make an entry in the input file like::

    sprite 75 {
        base = "ground_tiles/s75.png";
        top = 0;
        left = 0;
        width = 64;
        height = 32;
    }

This example says that sprite number 75 is in the ``ground_tiles/s75.png``
file (a 32bpp image file). The ``75`` in the file name is not a hard
requirement, but since there are many images, it is useful to give them a
systematic name, for easier retrieval and identification.

The ``top`` and ``left`` give the top-left pixel of the new sprite. The
``width`` and ``height`` give the horizontal and vertical size of the new
sprite. With sprite sheets, the ``top`` and ``left`` positions change between
sprites.

For new sprites that should also be recoloured, the entry looks a little
different::

    sprite 18 {
        base = "128_0004.png";
        left   = 0;
        top    = 0;
        width  = 128;
        height = 256;

        recolour = "128p_0004.png";
        layer 1 = 4;
        layer 2 = 3;
    }

The first part is the same. Sprite number 18 should be created from the
``128p_0004.png`` (32bpp RGBA) file. The new sprite is apparently 128x256
pixels big. The ``recolour`` entry points to the paletted image containing the
recolouring type information.

The easiest way to define recolouring is to use index values in the recolour
type
image that match with the CorsixTH recolouring requirements. If the recolour
indices in the image do not match with the CorsixTH requirements, you can use
``layer`` lines like ``layer 2 = 3;`` to change the encoded recolouring type.
The example layer line means that recolour type 2 in the image is changed to
recolour type 3 in the free graphics.

used different index values, or if the CorsixTH requirements changed after you
created the recolour image, the index values need to be adapted. Rather than
rewriting the recolour image, the encoder provides ``layer`` lines, like
``layer 1 = 3``. You can have several such lines, if necessary.

Once you have an input file with entries for each sprite that you want to
encode, it is a simple matter of running the ``encode`` program, like::

    encode input_file.txt foo.sprites

The ``encode`` program is started, it reads your new sprite entries in the
``input_file.txt`` file, loads all the ``.png`` files, and converts them to
the ``foo.sprites`` file. If CorsixTH is to use this file, the ``foo`` part
must be replaced by a name of a file in the original graphics (without file
extension).


Loading into CorsixTH
~~~~~~~~~~~~~~~~~~~~~
In the config file, you have to set ``use_new_graphics`` to ``true``, and
change ``new_graphics_folder`` to a directory that contains the files with
free graphics.

Each time CorsixTH loads a file, the program first loads the file with the
original sprites, and then it loads the file with the free graphics,
overwriting the already loaded original sprites. By doing it in this order,
the free graphics do not need to have all sprites, and the sprites that it
does have are used instead of the sprites of the original graphics.


Compiling
=========
The source code of the ``encode`` program used for encoding your images into a
free graphics file, can be found in the ``SpriteEncoder`` directory.

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
