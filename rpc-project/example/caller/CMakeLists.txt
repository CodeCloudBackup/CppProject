set(SRC_LIST call_user_service.cc ../user.pb.cc)
add_executable(consumer_user ${SRC_LIST})

target_link_libraries(consumer_user mprpc)
target_link_libraries(consumer_user protobuf)

set(SRC_LIST call_friend_service.cc ../friend.pb.cc)
add_executable(consumer_friend ${SRC_LIST})

target_link_libraries(consumer_friend mprpc)
target_link_libraries(consumer_friend protobuf)
