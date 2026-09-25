#%Module1.0
################################################################################
## rmap Environment Modulefile (Tcl Environment Modules)
## Hardware Register Map Designer & Model Generator
################################################################################

proc ModulesHelp { } {
    puts stderr "\nThe rmap module initializes environment paths for the rmap register map designer and model generator."
    puts stderr "Commands: rmap (GUI & CLI)\n"
}

module-whatis "Hardware Register Map Designer & Model Generator (C++17 / Qt 6)"

# Compute package root relative to modulefile location
set cur_dir [file dirname [file normalize [info script]]]
set pkg_root [file normalize "$cur_dir/../.."]

if { [info exists env(RMAP_ROOT)] } {
    set pkg_root $env(RMAP_ROOT)
}

prepend-path PATH "$pkg_root/bin"

if { [file isdirectory "$pkg_root/lib"] } {
    prepend-path LD_LIBRARY_PATH "$pkg_root/lib"
}

setenv RMAP_ROOT "$pkg_root"
setenv RMAP_TEMPLATES_DIR "$pkg_root/share/rmap/templates"
setenv RMAP_EXAMPLES_DIR "$pkg_root/share/rmap/examples"
setenv RMAP_DOCS_DIR "$pkg_root/share/rmap/doc"
