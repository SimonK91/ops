
set(OPSJLIB_SRC
  ops/AckData.java
  ops/BasicError.java
  ops/Channel.java
  ops/CommException.java
  ops/ConfigurationException.java
  ops/DataValidator.java
  ops/DeadlineMissedFilterQoSPolicy.java
  ops/DeadlineNotifier.java
  ops/DefaultDataValidator.java
  ops/DefaultOPSConfigImpl.java
  ops/DefaultSubscriberFilterQoSPolicy.java
  ops/Domain.java
  ops/Error.java
  ops/ErrorService.java
  ops/Event.java
  ops/FilterQoSPolicy.java
  ops/HeartBeat.java
  ops/InvalidHeaderException.java
  ops/KeyFilterQoSPolicy.java
  ops/Listener.java
  ops/Manager.java
  ops/McSendDataHandler.java
  ops/McUdpSendDataHandler.java
  ops/MemoryMap.java
  ops/MessageBuffer.java
  ops/MessageFilter.java
  ops/MessageFilterSet.java
  ops/MulticastReceiver.java
  ops/MulticastSender.java
  ops/MulticastSocketCreator.java
  ops/NetworkSupport.java
  ops/Notifier.java
  ops/OPSConfig.java
  ops/OPSConfigRepository.java
  ops/OPSEnum.java
  ops/OPSObject.java
  ops/OPSReceiveException.java
  ops/OPSTopicTypeMissmatchException.java
  ops/ParticipantFactory.java
  ops/ParticipantInfoData.java
  ops/ParticipantInfoDataListener.java
  ops/Participant.java
  ops/Publisher.java
  ops/QoSNotSupportedException.java
  ops/QualityOfService.java
  ops/QualityOfServicePolicy.java
  ops/ReadByteBuffer.java
  ops/ReceiveDataHandlerFactory.java
  ops/ReceiveDataHandler.java
  ops/ReceiverFactory.java
  ops/Receiver.java
  ops/ReceiveTimedOutException.java
  ops/ReliabilityQoS.java
  ops/Reply.java
  ops/Request.java
  ops/RequestReplyException.java
  ops/RequestReply.java
  ops/SendDataHandlerBase.java
  ops/SendDataHandlerFactory.java
  ops/SendDataHandler.java
  ops/Sender.java
  ops/Serializer.java
  ops/StaticManager.java
  ops/SubscriberFilterQoSPolicy.java
  ops/SubscriberHandlerNotFound.java
  ops/Subscriber.java
  ops/TcpClient.java
  ops/TcpClientReceiver.java
  ops/TcpSendDataHandler.java
  ops/TcpSenderList.java
  ops/TcpServerSender.java
  ops/TopicInfoData.java
  ops/TopicInterfaceData.java
  ops/TopicInterface.java
  ops/Topic.java
  ops/Transport.java
  ops/TypeHelper.java
  ops/UDPReceiver.java
  ops/UDPSender.java
  ops/WriteByteBuffer.java
  ops/XMLConfigLoader.java
  # archiver
  ops/archiver/BaseTestData.java
  ops/archiver/ChildTestData.java
  ops/archiver/OPSArchiverIn.java
  ops/archiver/OPSArchiverOut.java
  ops/archiver/OPSObjectFactory.java
  ops/archiver/TestTypesFactory.java
  # IOService
  ops/IOService/AsynchReceiveCallbackHandler.java
  ops/IOService/AsynchReceiveCallback.java
  ops/IOService/AsynchReceiveWork.java
  ops/IOService/IOService.java
  ops/IOService/MulticastEnableDatagramChannel.java
  ops/IOService/TestIOService.java
  # jni
  ops/jni/NativeByteBuffer.java
  ops/jni/TestNativeByteBuffer.java
  # mains test code
  ops/mains/TestInheritance.java
  # protocol
  ops/protocol/DataHeader.java
  ops/protocol/OPSMessage.java
  # transport
  ops/transport/inprocess/InProcessTransport.java
)

add_jar(OPSJLib ${OPSJLIB_SRC}
  INCLUDE_JARS ConfigurationLib
  VERSION ${OPS_VERSION}
  OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin
)

# in cmake-3.1 (at least) the install_jar is broken for Windows
if(UNIX)
  install_jar(OPSJLib ${OPS_JAR_REL_INSTALL_PATH})
else()
  # install manually
  set(JAR_OUTPUT ${CMAKE_BINARY_DIR}/bin/OPSJLib-${OPS_VERSION}.jar)
  install(FILES ${JAR_OUTPUT} DESTINATION ${OPS_JAR_REL_INSTALL_PATH})
  install(FILES ${JAR_OUTPUT} DESTINATION ${OPS_JAR_REL_INSTALL_PATH} RENAME OPSJLib.jar)
endif()

if(BUILD_DOCUMENTATION)
  create_javadoc(ops
    PACKAGES ops ops.archiver ops.protocol ops.jni ops.transport.inprocess
    SOURCEPATH "${CMAKE_CURRENT_SOURCE_DIR}"
    CLASSPATH ${CMAKE_JAVA_INCLUDE_PATH}
    WINDOWTITLE "OPSJLib documentation"
    DOCTITLE "<h1>OPSJLib documentation</h1>"
    AUTHOR TRUE
    USE TRUE
    VERSION TRUE
    )
endif()
