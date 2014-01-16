If the explanation in the ``README.txt`` file is a bit too short, below is a
longer discussion.

First, the requirements of the new sprites are discussed in `Making a new
sprite`_. How to convert the images to a file that can be used by Corsix-TH is
discussed in `Making a new sprite file`_.
Once you have made a new file, you need to know how to give it to Corsix-TH.
That is discussed in `Loading new sprite files into Corsix-TH`_.
Finally, it may be needed to compile the ``encode`` program (which actually
creates the new sprite files). How to do that is discussed in `Compiling`_.



Making a new sprite
===================
The first thing to do is to make one or more new sprites for the game items
that you want to replace with your graphics. Each sprite should have the same
size as the size of the original sprite, that is, have the same width and
height in pixels.

The new sprite should be provided as a full colour ``.png`` file, in RGBA
format, where each of the R, G, B, and A channels is 8 bit wide, giving a
total of 32 bit for a single pixel (commonly known as a 32bpp image).
If the sprite needs to be recoloured (for example, different colour clothes of
a patient), an additional 8 bit paletted image is needed of the same size as
the new sprite (or larger, but that is not so useful).
The colours of the palette are not relevant. Only the pixel index value is used.
Pixel value ``0`` in the paletted image means the RGBA data of the new sprite
at the same position should be used in the new sprite, that is, it means 'copy
the new sprite data'. Pixel values ``1`` to ``254`` mean 'recolour the pixel'.
It takes the maximum value of the R, G, and B channel values of the
corresponding pixel in the new sprite and uses it as index in the recolour
table with the same number as the pixel value. The A channel of the new sprite
is simply copied to the recoloured sprite.

The paletted image should not use pixel value ``255``, that table is reserved
for displaying the original sprites.


Making a new sprite file
========================
Once you have created one or more new sprites, they have to be put into a new
graphics file to be read by Corsix-TH. The ``encode`` program performs this
function. Here it is assumed that you have this program. If that is not the
case, you may want to create it first. How to do that is explained below, in
`Compiling`_ below.

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
file (a 32bpp image file). The ``top`` and ``left`` give the top-left pixel of
the new sprite. The ``width`` and ``height`` give the horizontal and vertical
size of the new sprite.

The coordinates may seem superfluous, but they make it possible to make a
single big image that contains many new sprites. Selecting the right sprite from
the image is then done by just setting the coordinates.

For new sprites that should be recoloured, the entry looks a little
different::

    sprite 4 {
        base = "128_0004.png";
        left   = 0;
        top    = 0;
        width  = 128;
        height = 256;

        recolour = "128p_0004.png";
        layer 1 = 4;
        layer 2 = 3;
    }

The first part is the same. Sprite 4 should be pulled from the
``128p_0004.png`` file. The new sprite is apparently 128x256 pixels big. The
``recolour`` entry points to the paletted image containing the recolouring
information. If the pixel indices in that paletted file are correct for
Corsix-TH, you are done with just the ``recolour`` line. However, suppose the
image was created to use recolour layer ``1`` and ``2``, but Corsix-TH wants
it to use layers ``4``, respectively ``3``. Lines like ``layer 1 = 4`` mean
that recolour table numbers in the paletted image are converted to another
number, in this case, pixel value ``1`` is converted to recolour table ``4``
in the program.


Once you have an input file with entries for each sprite that you want to
encode, it is a simple matter of running the ``encode`` program, like::

    encode input_file.txt output_file.sprites

The ``encode`` program is started, it reads your new sprite entries in the
``input_file.txt`` file, loads all the ``.png`` files, and converts them to
the ``output_file.sprites`` file. This file can be read by Corsix-TH.


Loading new sprite files into Corsix-TH
=======================================
The original sprites of Corsix-TH are distributed over several different
files. In the SDL2 version of the program, each these files can also have
files with new (free) sprites. Each time a file is loaded, the program first
loads the original sprites, and then it loads the files with the new sprites,
overwriting the already loaded sprites. In the config file, you have to set
``use_new_graphics`` to ``true``, and change ``new_graphics_folder`` to a
directory that contains the new sprite files.


Compiling
=========
The ``encoder`` program used for encoding your sprites into a data file that
can be read by Corsix-TH is provided in source code here in the
``SpriteEncoder`` directory. Before you can use the program, the source code
needs to be compiled.

Compile for use
---------------
What you need to do exactly depends on what you aim to do. If you are
interested in using the program as-is, you only need a C++ compiler (for
example ``g++``), and you need to have ``libpng`` installed.
Actual compilation is done like below::

    g++ -Wall -c -o parser.o  parser.cpp
    g++ -Wall -c -o scanner.o scanner.cpp
    g++ -Wall -c -o encode.o  encode.cpp
    g++ -Wall -c -o ast.o     ast.cpp
    g++ -Wall -c -o image.o   image.cpp
    g++ -Wall -c -o output.o  output.cpp

    g++ -Wall -o encoder parser.o scanner.o encode.o ast.o image.o output.o -lpng

The first six commands compile each of the ``.cpp`` file to its ``.o`` file.
The final command then links all ``*.o`` files together, adds the libpng
library (the ``-lpng`` at the end), and produces the ``encoder`` program.

Compile for development
-----------------------
If you took a look at the files in the ``SpriteEncoder`` directory, you'll
notice that some of the file look generated, while there are also files with
other extensions than ``.cpp`` or ``.h``.

In particular, there are ``scanner.l``, and ``parser.y``. These are input
files for the scanner generator ``lex`` (or the GNU implementation ``flex``),
respectively the parser generator ``yacc`` (or the GNU implementation
``bison``).

The scanner generator generates the ``scanner.cpp`` file, and the parser
generator generates the ``tokens.h`` and ``parser.cpp`` files. To allow
compiling the source code without having these generators, the generated files
have been added to the project as well.
Using the generators is done like::

    flex --outfile=scanner.cpp scanner.l
    bison --defines=tokens.h --output=parser.cpp parser.y

After running the generators, proceed with the normal C++ compilation, as
described above.


There are two other files in the ``SpriteEncoder`` directory that may be of
interest. The first file is ``decode.py``, which is a Python 3 implementation
of a decoder. At the time of writing, there were no recolour tables defined,
so it contains some arbitrary example tables. The decoder is not optimized, so
it should be easy to follow what is happening. The second file is ``mk``,
which is a shell script that I used during development. It contains the
commands explained above.


.. vim: tw=78 spell sw=4 sts=4
