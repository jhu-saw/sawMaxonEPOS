cmake_minimum_required (VERSION 3.16)
project (saw_maxon_epos_config VERSION 0.1.0)

find_package (catkin QUIET)
if (catkin_FOUND)
  catkin_package()
endif ()

install (
  DIRECTORY "${saw_maxon_epos_config_SOURCE_DIR}"
  DESTINATION share/sawMaxonEPOS
  COMPONENT mtsMaxonEPOS-Share
  PATTERN "CMakeLists.txt" EXCLUDE
  PATTERN "package.xml" EXCLUDE)
