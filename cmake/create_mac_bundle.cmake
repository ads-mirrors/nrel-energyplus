# Must set these variables:
# PROJECT_SOURCE_DIR
# APP_NAME
# ARGUMENT
# PRODUCTS_DIR

# Define paths
set(APP_BUNDLE_NAME "${APP_NAME}.app")
set(APP_DIR "${PRODUCTS_DIR}/${APP_BUNDLE_NAME}")
set(CONTENTS_DIR "${APP_DIR}/Contents")
set(MACOS_DIR "${CONTENTS_DIR}/MacOS")

# Ensure directories exist
file(MAKE_DIRECTORY "${CONTENTS_DIR}")
file(MAKE_DIRECTORY "${MACOS_DIR}")

configure_file(${PROJECT_SOURCE_DIR}/cmake/Shortcut.plist.in "${CONTENTS_DIR}/Info.plist" @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/cmake/launcher.sh.in "${MACOS_DIR}/launcher.sh" @ONLY)

execute_process(COMMAND chmod +x "${MACOS_DIR}/launcher.sh")

message(STATUS "Created macOS app bundle at ${APP_DIR}")
