cmake_minimum_required (VERSION 3.10)
project (compiz-elements)

find_package (Compiz REQUIRED)

include (CompizPlugin)
include (CompizCommon)
include (CompizPackage)

add_definitions (-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

include_directories (/tmp/compiz-dev-sysroot/usr/include /tmp/compiz-dev-sysroot/usr/include/compiz /tmp/compiz-dev-sysroot/usr/include/glibmm-2.4 /tmp/compiz-dev-sysroot/usr/lib/x86_64-linux-gnu/glibmm-2.4/include /tmp/compiz-dev-sysroot/usr/include/sigc++-2.0 /tmp/compiz-dev-sysroot/usr/lib/x86_64-linux-gnu/sigc++-2.0/include /tmp/compiz-dev-sysroot/usr/include/startup-notification-1.0)
link_directories (/tmp/compiz-dev-sysroot/usr/lib/x86_64-linux-gnu)
compiz_plugin (elements PKGDEPS compiz PLUGINDEPS composite opengl text)
target_link_libraries (elements GL)

set (COMPIZ_VERSION_MAJOR 0)
set (COMPIZ_VERSION_MINOR 9)
set (COMPIZ_VERSION_MICRO 4)
set (VERSION ${COMPIZ_VERSION_MAJOR}.${COMPIZ_VERSION_MINOR}.${COMPIZ_VERSION_MICRO})

compiz_add_plugins_in_folder ("${CMAKE_CURRENT_SOURCE_DIR}/plugins")

install (DIRECTORY images/ DESTINATION ${datadir}/compiz/elements/images)
install (DIRECTORY images/ DESTINATION ${datadir}/compiz/elements)

compiz_print_configure_header ("Compiz Elements Plugins")
compiz_print_plugin_stats ("${CMAKE_CURRENT_SOURCE_DIR}/plugins")
compiz_print_configure_footer ()

compiz_package_generation ("Compiz Elements Plugins")
compiz_add_uninstall ()
