# Keyboard Modifier Status for XFCE

*Keyboard Modifier Status for XFCE* is an XFCE port of the [Keyboard-Modifier-Status](https://github.com/sneetsher/Keyboard-Modifiers-Status) extension for Gnome Shell.

It provides a visual indication of the modifier keys currently pressed and is mostly useful if you're using sticky keys.

![modifier-status-screenshot](https://user-images.githubusercontent.com/91804/38436990-68cc0300-39d6-11e8-8229-ab7b4f56037b.png)

## Installation instructions

The plugin uses the [Meson Build System](http://mesonbuild.com/).

```bash
$ meson build
$ cd build
$ ninja && sudo ninja install
```

The plugin consists of 2 files:

  * `libxfce4-modifier-status-plugin.so`, installed to `/usr/lib/xfce4/panel/plugins` by default
  * `xfce4-modifier-status-plugin.desktop`, installed to `/usr/share/xfce4/panel/plugins` by default
