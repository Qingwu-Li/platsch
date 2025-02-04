platsch - Splash Screen Application
===================================

platsch is a simple splash screen application meant to be run as PID 1
(``init=/usr/sbin/platsch``).

The image to be displayed for each DRM connector is expected here::

  /usr/share/platsch/splash-<width>x<height>-<format>.bin

By default platsch uses the first mode on each DRM connector. ``<format>``
defaults to ``RGB565``. See below how to change that behavior.

Splash screen images must have the specified resolution and format. See
below how to generate them.

After displaying the splash screen(s), platsch forks, sending its child to
sleep to keep the DRM device open and the splash image(s) on the display(s).
Finally platsch gives PID 1 to ``/sbin/init``. Later another application can
simply take over.

Seamless transitions are possible (e.g. to *Weston* having the same image
configured as background). Depending on the SoC used, the same format might be
required to achieve that.

For questions, feedback, patches, please send a mail to::

  oss-tools@pengutronix.de

Note: you must be subscribed to post to this mailing list. You can do so by
sending an empty mail to ``oss-tools-subscribe@pengutronix.de``.

Splash Image Generation
-----------------------

*ImageMagick* can be used to prepare the required splash images. Since it
outputs "bottom-up" BMPs, flip them to compensate for that. Alpha channel is
enabled/disabled depending on whether RGB or XRGB format is desired. Finally
only the actual pixel data is extracted. Since a color profile might follow the
pixel data, strip that.

RGB565
^^^^^^

This generates a 1920x1080 splash image in ``RGB565`` format from a png file::

  #!/bin/bash
  convert \
    /path/to/source.png \
    -resize 1920x1080\! \
    -flip \
    -alpha off \
    -strip \
    -define bmp:subtype=RGB565 \
    bmp:- | tail -c $((1920*1080*(5+6+5)/8)) > \
    splash-1920x1080-RGB565.bin

XRGB8888
^^^^^^^^

This generates a 1920x1080 splash image in ``XRGB8888`` format from a png
file::

  #!/bin/bash
  convert \
    /path/to/source.png \
    -resize 1920x1080\! \
    -flip \
    -alpha on \
    -strip \
    bmp:- | tail -c $((1920*1080*(8+8+8+8)/8)) > \
    splash-1920x1080-XRGB8888.bin

Configuration
-------------

The directory searched for the splash images (default: ``/usr/share/platsch``),
as well as the image files' basename (default: ``splash``) can be controlled via
the environment variables ``platsch_directory`` and ``platsch_basename`` (which
in the case of PID != 1 would be overridden by the corresponding commandline
parameters, see further downwards).

For each connector a corresponding environment variable is looked up::

  platsch_<connector-type-name><connector-type-id>_mode

``<connector-type-name>`` is a lowercase string libdrm returns for
``drmModeGetConnectorTypeName()``, "-" replaced with "_". The variable is
expected to contain the mode's resolution and image format, optionally::

  <width>x<height>[@<format>]

I.e. to set the mode matching a resolution of 800x600 and default format on
``LVDS-1``::

  platsch_lvds1_mode=800x600

Or a resolution of 1920x1080 and ``XRGB8888`` format on ``LVDS-2``::

  platsch_lvds2_mode=1920x1080@XRGB8888

The kernel passes unrecognized key-value parameters not containing dots into
init's environment, see
`Kernel Parameter Documentation <https://www.kernel.org/doc/html/latest/admin-guide/kernel-parameters.html>`_.
Therefore the above settings can be supplied via the kernel commandline. This
also allows dynamic use cases where the bootloader decides which resolution/mode
to use on which connector.

Commandline Arguments
---------------------

If the program is used in later Linux userspace (PID != 1), platsch recognizes a
couple of command line arguments:

``--directory`` or ``-d`` sets the directory containing the splash screens.

``--basename`` or ``-b`` sets the prefix of the splash screen file names.

Contributing
------------

The Git repository for this software can be found at::

  https://git.pengutronix.de/cgit/platsch

Any patches should be sent to the mailing list above. Please prefix your
subject with "[PATCH platsch]" (when sending patches with Git, see the
git-config manpage for the option ``format.subjectPrefix``).
Mails sent to this mailing list are also archived at::

  https://lore.pengutronix.de/oss-tools

This project uses the Developer's Certificate of Origin, as stated in the file
DCO bundled with this software, using the same process as for the Linux kernel::

  https://www.kernel.org/doc/html/latest/process/submitting-patches.html#sign-your-work-the-developer-s-certificate-of-origin

By adding a Signed-off-by line (e.g. using ``git commit -s``) saying::

  Signed-off-by: Random J Developer <random@developer.example.org>

(using your real name and e-mail address), you state that your contributions
are in line with the DCO.

Cross compiling instructions
----------------------------

To cross-compile the project, use the following commands:

.. code-block:: shell

    meson setup --cross-file=<path-to-meson-cross-file>
    ninja -C build

Here are sample cross commands：

.. code-block:: shell

    meson setup ./build -DHAVE_CAIRO=true --cross-file ./meson.cross
    ninja -C build

Here is a sample cross file:

.. code-block:: ini

    [binaries]
    c = ${CC}
    cpp = ${CXX}
    cython = 'cython3'
    ar = '${AR}'
    nm = '${NM}'
    strip = '${STRIP}'
    readelf = '${READELF}'
    objcopy = '${OBJCOPY}'
    pkgconfig = '${pkgconfig}'

    [properties]
    needs_exe_wrapper = true


    [target_machine]
    system = 'linux'
    cpu_family = 'aarch64'
    cpu = 'aarch64'
    endian = 'little'


Build options
-------------

The following build options are available:

.. list-table::
   :header-rows: 1

   * - Option name
     - Values
     - Default
     - Notes
   * - HAVE_CAIRO
     - true, false
     - true
     - Enable Cairo support
   * - SPINNER
     - true, false
     - false
     - Enable spinner

Spinner - Splash Screen with Animation
======================================

The `spinner` executable is designed to provide boot animations. It supports two types of animations:

1. **Square PNG Rotation Animation**: Rotates a square PNG image.
2. **Sequence Move Rectangle Animation**: Displays a sequence of square images from a strip of PNG images.

spinner Configuration
---------------------

The configuration for the `spinner` executable is read from a configuration file,
with a default path of `/usr/share/platsch/spinner.conf`.
The directory of the configuration file can be set via the `platsch_directory` environment variable.

Example Configuration File
--------------------------

Here is an example of a configuration file (`spinner.conf`):

.. code-block:: ini

    backdrop="/usr/share/platsch/splash.png"
    symbol="/usr/share/platsch/Spinner.png"
    fps=20
    # frames=0 means infinite
    frames=0
    text="text to display"
    text_x=350
    text_y=400
    text_font="Sans"
    text_size=30
