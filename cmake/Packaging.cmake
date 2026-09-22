# This file is part of rmap.
#
# rmap is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# rmap is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with rmap.  If not, see <https://www.gnu.org/licenses/>.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

# CPack Packaging Configuration for rmap

set(CPACK_PACKAGE_NAME "rmap")
set(CPACK_PACKAGE_VENDOR "Ezequiel Alves")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Hardware Register Map Designer & Model Generator")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/zkalves/rmap")
set(CPACK_PACKAGE_CONTACT "Ezequiel Alves <https://github.com/zkalves/rmap>")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

set(CPACK_PACKAGE_VERSION "${RMAP_VERSION_SEMVER}")
set(CPACK_PACKAGE_VERSION_MAJOR "${RMAP_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${RMAP_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${RMAP_VERSION_PATCH}")

set(CPACK_STRIP_FILES TRUE)
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
set(CPACK_GENERATOR "DEB;RPM;TGZ")
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}/packages")


# Debian (.deb) Configuration
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Ezequiel Alves <https://github.com/zkalves/rmap>")
set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

# RPM (.rpm) Configuration
set(CPACK_RPM_PACKAGE_LICENSE "GPL-3.0-or-later")
set(CPACK_RPM_PACKAGE_GROUP "Development/Tools")
set(CPACK_RPM_PACKAGE_URL "https://github.com/zkalves/rmap")
set(CPACK_RPM_PACKAGE_AUTOREQPROV ON)
set(CPACK_RPM_FILE_NAME RPM-DEFAULT)

# Archive & Tarball naming
set(CPACK_ARCHIVE_FILE_NAME "rmap-${RMAP_VERSION_SEMVER}-Linux")

# Ignore patterns for source packages
set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")
set(CPACK_SOURCE_IGNORE_FILES
    "/.git/"
    "/build/"
    "/work/"
    "/_site/"
    ".*~$"
    "\\\\.pyc$"
    "__pycache__"
)

include(CPack)
