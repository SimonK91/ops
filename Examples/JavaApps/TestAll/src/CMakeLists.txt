
set(APP_SRC
  testallmain.java
  VerifySerDes.java
)

# generate a MANIFEST file
configure_file(MANIFEST.MF.in MANIFEST.MF)

add_jar(VerifySerDesJava ${APP_SRC}
  INCLUDE_JARS ${PROJECT_BINARY_DIR}/bin/TestAll.jar ${OPSJLIB_JAR} ${CONFIGURATIONLIB_JAR}
  ENTRY_POINT VerifySerDes
  MANIFEST ${CMAKE_CURRENT_BINARY_DIR}/MANIFEST.MF
  VERSION ${PROJECT_VERSION}
  OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin
)

add_dependencies(VerifySerDesJava TestAll)

install(FILES
  ${CMAKE_BINARY_DIR}/bin/VerifySerDesJava-${PROJECT_VERSION}.jar
  ${OPSJAR_PATH}/OPSJLib-${PROJECT_VERSION}.jar
  ${OPSJAR_PATH}/ConfigurationLib-${PROJECT_VERSION}.jar
  ${PROJECT_BINARY_DIR}/bin/TestAll.jar
  DESTINATION examples/testall
)
