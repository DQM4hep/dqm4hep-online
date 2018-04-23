#######################################################
# cmake file for building DQMOnline package
# @author Eté Rémi
# Copyright (c)
#######################################################

# include directories
include_directories( BEFORE include )
dqm4hep_set_cxx_flags()

#AUX_SOURCE_DIRECTORY( src SRC_FILES )
SET( SRC_FILES ${SRC_FILES} src/RemoteLogger.cc )
SET( SRC_FILES ${SRC_FILES} src/OnlineManagerServer.cc )
SET( SRC_FILES ${SRC_FILES} src/OnlineRoutes.cc )
SET( SRC_FILES ${SRC_FILES} src/EventSource.cc )
SET( SRC_FILES ${SRC_FILES} src/RunControl.cc )
SET( SRC_FILES ${SRC_FILES} src/RunControlServer.cc )
SET( SRC_FILES ${SRC_FILES} src/RunControlInterface.cc )
SET( SRC_FILES ${SRC_FILES} src/AppEvent.cc )
SET( SRC_FILES ${SRC_FILES} src/AppEventLoop.cc )
SET( SRC_FILES ${SRC_FILES} src/Application.cc )
SET( SRC_FILES ${SRC_FILES} src/EventCollector.cc )

# ----- Http run control based on Mongoose library -----
if( BUILD_HTTP_RC )
  dqm4hep_find_package(
    PACKAGE Mongoose
    REQUIRED ADD_DEPENDS  
  )
  set( SRC_FILES ${SRC_FILES} src/plugins/HttpRunControlPlugin.cc )
endif()

# 
add_shared_library( ${PROJECT_NAME} ${SRC_FILES} )
install( TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib )
install_directory( include DESTINATION . FILES_MATCHING PATTERN "*.h" PATTERN "*.cc" )

# -------- Add executables --------
dqm4hep_add_executable( main dqm4hep-start-rc-server bin )
dqm4hep_add_executable( main dqm4hep-start-random-event-source bin )
dqm4hep_add_executable( main dqm4hep-start-online-mgr bin )
dqm4hep_add_executable( main dqm4hep-online-logger bin )
dqm4hep_add_executable( main dqm4hep-start-event-collector bin )

# -------- clang tidy and format --------
dqm4hep_run_clang_tidy()
dqm4hep_run_clang_format()

