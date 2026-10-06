cmake_minimum_required (VERSION 3.10)
project (compiz-stackswitch)

find_package (Compiz REQUIRED)

include (CompizPlugin)
include (CompizCommon)
include (CompizPackage)

add_definitions (-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

include_directories (/tmp/compiz-dev-sysroot/usr/include /tmp/compiz-dev-sysroot/usr/include/compiz /tmp/compiz-dev-sysroot/usr/include/glibmm-2.4 /tmp/compiz-dev-sysroot/usr/lib/x86_64-linux-gnu/glibmm-2.4/include /tmp/compiz-dev-sysroot/usr/include/sigc++-2.0 /tmp/compiz-dev-sysroot/usr/lib/x86_64-linux-gnu/sigc++-2.0/include /tmp/compiz-dev-sysroot/usr/include/startup-notification-1.0)
link_directories (/tmp/compiz-dev-sysroot/usr/lib/x86_64-linux-gnu)
compiz_plugin (stackswitch PLUGINDEPS opengl composite text)
target_link_libraries (stackswitch GL)
