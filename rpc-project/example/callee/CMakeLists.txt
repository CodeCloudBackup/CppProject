set(SRC_LIST user_service.cc ../user.pb.cc)
add_executable(provider_user ${SRC_LIST})

target_link_libraries(provider_user mprpc)
target_link_libraries(provider_user protobuf)

set(SRC_LIST friend_service.cc ../friend.pb.cc)
add_executable(provider_friend ${SRC_LIST})

target_link_libraries(provider_friend mprpc)
target_link_libraries(provider_friend protobuf)