/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package name.guillaumes.jordi.jblinkenserver;

import java.io.ByteArrayInputStream;
import java.io.DataInputStream;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.SocketTimeoutException;

/**
 *
 * @author jguillaumes
 */
public class JBlinkenServer {

    private static final short FLG_RESYNC = 0x01;
    private static final short FLG_DETACH = 0x02;
    private static JBlinkenPanel jbp;
    private boolean toExit = false;
    private DatagramSocket socket = null;

    public void setToExit() {
        toExit = true;
    }
   
    public void listener() {
        byte[] buffer = new byte[4096];
        byte function = 0;
        byte flags    = 0;
        long sequence, 
          oldsequence = 0;
        short bflags  = 0;
        int numBytes  = 0;
        DataInputStream is = null;
        boolean usePacket = false;

        InetAddress pair = null;

        DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
        try {
            socket = new DatagramSocket(11696);
            socket.setSoTimeout(10000);
        } catch (SocketException ex) {
            throw new RuntimeException(ex);
        }
        while (! toExit) {
            try {
                socket.receive(packet);
                // System.err.println("Got a packet, " + packet.getLength() + " bytes long, offset " + packet.getOffset());
                usePacket = false;
                is = new DataInputStream(new ByteArrayInputStream(packet.getData()));
                function = is.readByte();
                flags    = is.readByte();
                sequence = is.readInt();
                bflags   = is.readShort();
                numBytes = is.readUnsignedShort();
                if (pair != null) {
                    if (pair.equals(packet.getAddress())) {
                        if (sequence > oldsequence) {
                            oldsequence = sequence;
                            usePacket = true;
                        } else if ((flags & FLG_RESYNC) != 0) {
                            oldsequence = sequence;
                            usePacket = true;
                        } else {
                            System.err.println("Packet out of sequence " + sequence + " vs " + oldsequence);
                        }
                    }
                } else {
                    pair = packet.getAddress();
                    oldsequence = sequence;
                    usePacket = true;
                }
                if (usePacket) {
                    if (numBytes != 2) {
                        System.err.println("Error, unsupported " + numBytes + " bytes packet.");
                    } else {
                        int theNum = is.readUnsignedShort();
                        jbp.setLights(bflags, theNum);
                    }
                }
            } catch (SocketTimeoutException ste) {
              pair = null;
              oldsequence = 0;
              System.err.println("Timeout reading packet, unbinding");
            } catch (IOException ex) {
                System.err.println("IOError reading socket");
                throw new RuntimeException(ex);
            }
        }
    }

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        JBlinkenServer jbs = new JBlinkenServer();
        jbp = new JBlinkenPanel(jbs);
        java.awt.EventQueue.invokeLater(new Runnable() {

            @Override
            public void run() {
                jbp.setVisible(true);
            }
        });
        jbs.listener();
    }
}
