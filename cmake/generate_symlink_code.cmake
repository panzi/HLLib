# Set a variable with CMake code which:
# Creates a symlink from src to dest (if possible) or alternatively
# copies if different.
macro(generate_symlink_code CODE SRC DEST WORKING_DIR)
	if(WIN32 AND NOT CYGWIN)
		set(_generate_symlink_code_message "Copying ${SRC} to ${DEST} if needed")
		set(_generate_symlink_code_operation "copy_if_different")
	else()
		set(_generate_symlink_code_message "Symlinking ${SRC} to ${DEST}")
		set(_generate_symlink_code_operation "create_symlink")
	endif()

	set(${CODE} "
		message(STATUS \"${_generate_symlink_code_message}\")
		execute_process(COMMAND \${CMAKE_COMMAND} -E ${_generate_symlink_code_operation}
			\"${SRC}\" \"${DEST}\" WORKING_DIRECTORY \"${WORKING_DIR}\")
		")
endmacro()
