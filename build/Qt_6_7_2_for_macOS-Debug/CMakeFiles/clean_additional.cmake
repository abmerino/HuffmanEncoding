# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/HuffmanEncoding_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/HuffmanEncoding_autogen.dir/ParseCache.txt"
  "HuffmanEncoding_autogen"
  )
endif()
