#!/usr/bin/python -W ignore::DeprecationWarning
import sys
import gtk

def get_pixel_rgb(x, y):
    pixbuf = gtk.gdk.Pixbuf(gtk.gdk.COLORSPACE_RGB, False, 8, 1, 1)
    pixbuf.get_from_drawable(gtk.gdk.get_default_root_window(),
                             gtk.gdk.colormap_get_system(),
                             x, y, 0, 0, 1, 1)
    return pixbuf.get_pixels_array()[0][0]

print(get_pixel_rgb(int(sys.argv[1]), int(sys.argv[2])))
