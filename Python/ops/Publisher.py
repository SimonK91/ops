import ops.Participant
from ops.opsTypes import Message

from ops.OPS_Archiver import OPS_Archiver_Out
from ops.DataAssembly import Segment
from ops.Constants import *

class Publisher(object):
	@staticmethod
	def chunkstring(string, length, limit):
		return [string[0+i:min(length,limit+i)] for i in range(0, length, limit)]

	@staticmethod
	def create(domainID,topicname):
		return Publisher(ops.Participant.Participant.getInstance(domainID).createTopic(topicname))

	def __init__(self,topic):
		super(Publisher, self).__init__()
		self.topic = topic
		self.key=""
		self.name=""

		self.message = Message()
		self.message.publisherName = self.name
		self.message.topicName = topic.name

		self.currentPublicationID=0
		self.participant = ops.Participant.Participant.getInstance(topic.domainID, topic.participantID)
		self.sendDataHandler = None
		self.start()

	def write(self,data,doValidation = True):
		if doValidation:
			data.validate()

		if self.key != "":
			data.key=self.key
		self.message.data = data
		self.message.publisherName = self.name
		self.message.publicationID = self.currentPublicationID;

		archiver = OPS_Archiver_Out(self.topic.sampleMaxSize,self.topic.optNonVirt)
		archiver.Ops("message",self.message)
		archiver.Spare(data.spareBytes)

		blocks = Publisher.chunkstring(archiver.buffer,archiver.index,PACKET_MAX_SIZE - SEGMENTSIZE)
		for i in range(len(blocks)):
			segment = Segment()
			segment.NumberOfSegments = len(blocks)
			segment.currentSegment = i
			block = segment.serialize() + blocks[i]
			self.sendDataHandler.sendData(block,self.topic)
		self.currentPublicationID+=1

	def start(self):
		if self.sendDataHandler is None:
			self.sendDataHandler = self.participant.getSendDataHandler(self.topic)
			self.sendDataHandler.addPublisher(self)

	def stop(self):
		if self.sendDataHandler is not None:
			self.sendDataHandler.removePublisher(self)
			self.participant.releaseSendDataHandler(self.topic)
			self.sendDataHandler = None

	def getTopic(self):
		return self.topic

