
set(APP_SRC
  pizzatestimpl/Main.java
)

# generate a MANIFEST file
configure_file(MANIFEST.MF.in MANIFEST.MF)

add_jar(PizzaTestImpl ${APP_SRC}
  INCLUDE_JARS ${PROJECT_BINARY_DIR}/bin/PizzaProject.jar ${OPSJLIB_JAR} ${CONFIGURATIONLIB_JAR}
  ENTRY_POINT pizzatestimpl/Main
  MANIFEST ${CMAKE_CURRENT_BINARY_DIR}/MANIFEST.MF
  VERSION ${PROJECT_VERSION}
  OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin
)

add_dependencies(PizzaTestImpl PizzaProject)

install(FILES
  ${CMAKE_BINARY_DIR}/bin/PizzaTestImpl-${PROJECT_VERSION}.jar
  ${OPSJAR_PATH}/OPSJLib-${PROJECT_VERSION}.jar
  ${OPSJAR_PATH}/ConfigurationLib-${PROJECT_VERSION}.jar
  ${PROJECT_BINARY_DIR}/bin/PizzaProject.jar
  DESTINATION examples/pizzatest
)
