
set(PUB_NAME RequestHelloPub)
set(SUB_NAME RequestHelloSub)

include_directories(${OPSIDL_SOURCE_DIR}/HelloRequestReply/Generated/Cpp)

#
add_executable(${PUB_NAME} RequestHelloTopic_pub.cpp)
add_executable(${SUB_NAME} RequestHelloTopic_sub.cpp)

# generate HelloRequestReply ops idl project if necessary
add_dependencies(${PUB_NAME} HelloRequestReply)
add_dependencies(${SUB_NAME} HelloRequestReply)

set_target_properties(${PUB_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${SUB_NAME} PROPERTIES VERSION ${PROJECT_VERSION})

target_link_libraries(${PUB_NAME} ${Boost_LIBRARIES} ops-static)
target_link_libraries(${SUB_NAME} ${Boost_LIBRARIES} ops-static)

if(MINGW)
  target_link_libraries(${PUB_NAME} ws2_32 mswsock)
endif()

install(TARGETS ${PUB_NAME} ${SUB_NAME} DESTINATION examples/requesthello)

file(INSTALL ${OPSIDL_SOURCE_DIR}/HelloRequestReply/src/ops_config.xml DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/requesthello)