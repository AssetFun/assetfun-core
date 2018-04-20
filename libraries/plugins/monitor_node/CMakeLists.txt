file(GLOB HEADERS "include/graphene/monitor_node/*.hpp")

add_library( graphene_monitor 
             monitor_plugin.cpp
             op_monitor.cpp
             coin_feed_price_monitor.cpp
             coin_object_monitor.cpp
             witness_monitor.cpp
           )

target_link_libraries( graphene_monitor graphene_chain graphene_app )
target_include_directories( graphene_monitor
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

if(MSVC)
  set_source_files_properties( monitor_plugin.cpp PROPERTIES COMPILE_FLAGS "/bigobj" )
endif(MSVC)

install( TARGETS
   graphene_monitor

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
INSTALL( FILES ${HEADERS} DESTINATION "include/graphene/monitor" )

