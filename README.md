##Glass Window Manager

### About
Glass is a small, extensible, window manager.  Its design is highly object-oriented and its use of interfaces allows for modular components with the option of drop-in replacements.  The major components of a window manager are each represented by a single interface, allowing a user to change behavior by selecting a different implementation, or by writing a new one.

For instance, any window layout can be supported by reimplementing the `WindowLayout` interface, and any display server can be supported by reimplementing the `DisplayServer` interface.  In fact, the majority of the program is completely display-server-agnostic, and wouldn't know or care if it's running on X or Wayland (though implementations for Wayland have not yet been written).

### Source
Check out the source by cloning the repository:

    $ git clone https://github.com/cdbfoster/glass.git

### Configuration
Glass is configured by editing the source itself.  Many common options including key bindings, window decoration settings, interface fonts, and layout selection are present in `source/config.hpp`.

### Building
Glass uses CMake as its build system.  It uses C++11, and its library dependencies are listed in the root `CMakeLists.txt`.

In-source builds are not allowed, so make a `build` directory outside of the repository:

    $ mkdir build
    $ cd build
    $ cmake ../glass
    $ make

This will produce the `glass-wm` executable.

There is also a [PKGBUILD available](https://aur.archlinux.org/packages/glass-wm-git/) in the AUR for Arch Linux users.

### Use
The easiest way to use Glass is to add `exec glass-wm` to `~/.xinitrc`.  This assumes that `glass-wm` can be found through your `$PATH`.

If using the `Default_WindowDecorator` implementation, the text in the bottom left-hand corner of the screen (defaults to "Glass") is read from the root window's name, which can be set using `xsetroot -name "<name here>"`.

### Contact
Questions and comments can be sent to my email, cdbfoster@gmail.com.

© 2014 - 2015 Christopher Foster
