#Glass Window Manager
Glass is a small, extensible, window manager.  Its design is highly object-oriented and its use of interfaces allows for modular components with the option of drop-in replacements.  The major components of the window manager are each represented by a single interface, allowing a user to change behavior by selecting a different implementation, or by writing a new one.

For instance, any window layout can be supported by reimplementing the `WindowLayout` interface, and any display server can be supported by reimplementing the `DisplayServer` interface.  In fact, the majority of the program is completely display-server-agnostic, and wouldn't know or care if it's running on X or Wayland (though an implementation for Wayland has not yet been written).

## Source
Check out the source by cloning the repository:

    $ git clone https://github.com/cdbfoster/glass.git

The source was written using a tab width of 4.

## Configuration
Glass is configured by editing the source itself.  Many common options including key bindings, window decoration settings, interface fonts, and layout selection are present in `source/config.hpp`.

## Building
Glass uses CMake as its build system.  It uses C++11, and its library dependencies are listed in the root `CMakeLists.txt`.  Instructions for individual distributions are below.

In-source builds are not allowed, so make a `build` directory outside of the repository:

    $ mkdir build
    $ cd build
    $ cmake ../glass
    $ make

This will produce the `glass-wm` executable.

### Arch Linux
Glass can be built using the [`glass-wm-git`](https://aur.archlinux.org/packages/glass-wm-git/) package from the AUR.

### Ubuntu derivates
You'll need the `*-dev` packages of each library in order to build Glass.  That should be possible with:

    $ sudo apt-get install libcairo2-dev libpango1.0-dev libx11-dev libxcb1-dev libxcb-cursor-dev libxcb-ewmh-dev libxcb-icccm4-dev libxcb-keysyms1-dev libxcb-util0-dev

## Use
The easiest way to use Glass is to add `exec glass-wm` to `~/.xinitrc`.  This assumes that `glass-wm` can be found through your `$PATH`.

If using the `Default_WindowDecorator` implementation, the text in the bottom left-hand corner of the screen (defaults to "Glass") is read from the root window's name, which can be set using `xsetroot -name "<name here>"`.

By default, the `Default_WindowDecorator` implementation provides transparent decorations.  Use a compositor like `xcompmgr` to render them properly.

### Key Bindings
Key bindings are configured in `source/config.hpp`.  The defaults are listed below:

#### Window Actions
Action | Key
---|---
Modal move | Super + Left Mouse Button
Modal resize | Super + Right Mouse Button
Close | Super + Q
Toggle Raised | Super + Up Arrow
Toggle Lowered | Super + Down Arrow
Toggle Fullscreen | Super + M

#### Focus Actions
Action | Key
---|---
Cycle Left | Super + Left Arrow
Cycle Right | Super + Right Arrow
Switch Tabbed | Super + Tab

#### Layout Actions
Action | Key
---|---
Toggle Floating | Super + Enter
Raise Floating | Super
Next Layout | Super + Control + Right Arrow
Previous Layout | Super + Control + Left Arrow

#### Tag Actions
Action | Key
---|---
Set Tag | Super + [0 - 9]
Toggle Tag | Super + Shift + [0 - 9]
Send Client to Tag | Super + Control + [0 - 9]
Toggle Client on Tag | Super + Control + Shift + [0 - 9]

#### Spawn Commands
Command | Key
---|---
xterm | Super + T
gedit | Super + E
firefox | Super + B
dbus-launch thunar | Super + F
dmenu_run | Super + Spacebar

#### Miscellaneous Actions
Action | Key
---|---
Quit Glass | Super + Shift + Q

## Contact
Questions and comments can be sent to my email, cdbfoster@gmail.com.

© 2014 - 2015 Chris Foster
