# Define file paths
set(FRONTEND_DIR "${CMAKE_CURRENT_LIST_DIR}/frontend")
set(FRONTEND_INDEX "${FRONTEND_DIR}/dist/index.html")
set(FRONTEND_BUNDLE "${FRONTEND_DIR}/dist/bundle.js")

# Check if the files exist
if(NOT EXISTS ${FRONTEND_INDEX} OR NOT EXISTS ${FRONTEND_BUNDLE})
    message(STATUS "Frontend files not found. Running 'pnpm run build'...")
    
    execute_process(
        COMMAND pnpm install
        WORKING_DIRECTORY ${FRONTEND_DIR}
        RESULT_VARIABLE INSTALL_RESULT
    )
    
    if(NOT INSTALL_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to execute 'pnpm install'. Check if pnpm is installed.")
    endif()
    
    execute_process(
        COMMAND pnpm build
        WORKING_DIRECTORY ${FRONTEND_DIR}
        RESULT_VARIABLE BUILD_RESULT
    )
    
    if(NOT BUILD_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to execute 'pnpm build'. Check the errors above.")
    endif()
    
    message(STATUS "Frontend build completed successfully!")
endif()

idf_component_register(
  SRCS "web_server.c" "digital_output.c"
  INCLUDE_DIRS "include"
  PRIV_REQUIRES esp_wifi esp_http_server digital_output json
  EMBED_FILES ./frontend/dist/index.html ./frontend/dist/bundle.js
)
