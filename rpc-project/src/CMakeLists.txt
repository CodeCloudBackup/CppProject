file(GLOB SRC_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
    
add_library(mprpc SHARED ${SRC_LIST})

target_link_libraries(mprpc muduo_net)
target_link_libraries(mprpc muduo_base)
target_link_libraries(mprpc pthread)
target_link_libraries(mprpc zookeeper_mt)
