/**
*
* Copyright (C) 2006-2009 Anton Gravestam.
*
* This file is part of OPS (Open Publish Subscribe).
*
* OPS (Open Publish Subscribe) is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* OPS (Open Publish Subscribe) is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with OPS (Open Publish Subscribe).  If not, see <http://www.gnu.org/licenses/>.
*/
package ops;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.logging.Level;
import java.util.logging.Logger;
import ops.archiver.OPSArchiverOut;
import ops.protocol.OPSMessage;

/**
 *
 * @author Anton Gravestam
 */
public class Publisher 
{
    private Topic topic;
    private int currentPublicationID = 1;
    private String name = "";
    private String key = "";
    private int reliableWriteNrOfResends = 1;
    private int reliableWriteTimeout = 1000;
    private byte[] bytes;
    private ByteBuffer buffer;
    private Participant participant;
    private final SendDataHandler sendDataHandler;

    public Publisher(Topic topic) throws CommException
    {
        this.topic = topic;
        bytes = new byte[topic.getSampleMaxSize()];
        buffer = ByteBuffer.allocateDirect(topic.getSampleMaxSize());
            
        this.participant = Participant.getInstance(topic.getDomainID(), topic.getParticipantID());
        sendDataHandler = participant.getSendDataHandler(topic);

    }
    protected void write(OPSObject o)
    {

        OPSMessage message = new OPSMessage();
        o.setKey(key);
        message.setData(o);
        message.setPublicationID(currentPublicationID);
        message.setTopicName(topic.getName());
        message.setPublisherName(name);

        WriteByteBuffer buf = new WriteByteBuffer(buffer, StaticManager.MAX_SIZE);
        try
        {

            //buf.writeProtocol();
            //buf.write("");
            //buf.write(1);
            //buf.write(0);

            buffer.position(0);
            OPSArchiverOut archiverOut = new OPSArchiverOut(buf);
        
            archiverOut.inout("message", message);

            //If o has spare bytes, write them to the end of the buf
            if(o.spareBytes.length > 0)
            {
                buf.write(o.spareBytes, 0, o.spareBytes.length);
            }
            //Finish will in nrOf segments in all segments.
            buf.finish();
            int sizeToSend = buffer.position();

            //transport.send(archiverOut.getBytes());
            buffer.position(0);
            buffer.get(bytes);
            sendDataHandler.sendData(bytes,sizeToSend, topic);
            //sendDataHandler.sendData(buffer.array(), buf.position(), topic);

        }
        catch (IOException ex)
        {
            Logger.getLogger(Publisher.class.getName()).log(Level.SEVERE, null, ex);
            ex.printStackTrace();
        }
        incCurrentPublicationID();
        
        
    }
    
    public void writeAsOPSObject(OPSObject o)
    {
        write(o);
    }

    private void incCurrentPublicationID()
    {
        if(currentPublicationID < 10000)
            currentPublicationID ++;
        else
            currentPublicationID = 1;
    }

    public String getName()
    {
        return name;
    }

    public void setName(String name)
    {
        this.name = name;
    }

    public Topic getTopic()
    {
        return topic;
    }

    public String getKey()
    {
        return key;
    }

    public void setKey(String key)
    {
        this.key = key;
    }

    public int getCurrentPublicationID()
    {
        return currentPublicationID;
    }
    
    

    public int getReliableWriteTimeout()
    {
        return reliableWriteTimeout;
    }

    public void setReliableWriteTimeout(int reliableWriteTimeout)
    {
        this.reliableWriteTimeout = reliableWriteTimeout;
    }
        

    public int getReliableWriteNrOfResends()
    {
        return reliableWriteNrOfResends;
    }

    public void setReliableWriteNrOfResends(int reliableWriteNrOfResends)
    {
        this.reliableWriteNrOfResends = reliableWriteNrOfResends;
    }


}
