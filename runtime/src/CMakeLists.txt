# Gather our source files in this directory.
set(
  vcalc_rt_files
  "${CMAKE_CURRENT_SOURCE_DIR}/placeholder.c"
)

# Build our executable from the source files.
add_library(vcalcrt SHARED ${vcalc_rt_files})
target_include_directories(vcalcrt PUBLIC ${RUNTIME_INCLUDE})

# Symbolic link our library to the base directory so we don't have to go searching for it.
symlink_to_bin("vcalcrt")
