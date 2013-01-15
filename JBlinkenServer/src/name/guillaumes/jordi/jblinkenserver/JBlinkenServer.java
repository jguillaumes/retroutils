/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package name.guillaumes.jordi.jblinkenserver;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.ByteArrayInputStream;
import java.io.DataInputStream;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.SocketTimeoutException;
import java.text.NumberFormat;
import javax.swing.Timer;

/**
 * Software implementation of the PDP-11 console "data lights".
 * This class is the controller. It needs JBlinkenPanel, which contains the
 * GUI. At this time it is really simple: just an array of 18 "LEDs". From
 * left to right, those leds are:
 * - Error led (red)
 * - Parity led (orange)
 * - Data leds (16, from most to less significant bit)
 * 
 * This class listens to incoming packets from an UDP port. By default, that
 * is port 11696, but it can be changed using the GUI (no persistent preferences
 * at this time).
 * 
 * The packet format is the same which is used by the "hardware" implementation
 * of the blinken lights console. That is:
 * 
 * - 1 byte of command code. Right this field is disregarded.
 * - 1 byte of packet flags:
 *   * 0x0001 indicates the server it has to disregard the sequence number and
 *          take whatever comes in this packet.
 *   * 0x0002 indicates the server it has to "unbound" itself from the current
 *          client (this is not yet implemented).
 * - 4 bytes of sequence number. The server discards any packets which come
 *     out of order.
 * - 2 bytes of message flags:
 *     * 0x0001 indicates the server it should not use the parity light
 *     * 0x0002 requires the server to disregard the data bytes and to
 *              put on the error light
 *     * 0x0004 requires the server to disregard the data bytes and to 
 *              put on ALL the lights (test mode)
 * - 2 bytes indicating the number of bytes of actual lights. Right now it
 *     must be 2 (two).
 * - Up to 32 data bytes (only two are used now).
 * 
 * All the integer values are in network order (big endian).
 * 
 * @author jguillaumes
 */
public class JBlinkenServer {

    /*
     * Constants for flags and status codes
     */
    private static final short FLG_RESYNC = 0x01;
    private static final short FLG_DETACH = 0x02;
    public static final short BLF_NOPARITY = 0x0001;
    public static final short BLF_ERROR = 0x0002;
    public static final short BLF_TEST = 0x0004;
    private static final int BLS_DEFAULT_PORT = 11696;
    private static final int BLS_DEFAULT_TIMEOUT = 10000;
    
    /*
     * GUI panel and UDP listener thread handler
     */
    private JBlinkenPanel jbp = null;
    private JbsUdpListener jbudp = null;
    
    /*
     * Counters
     */
    private int currentPort = BLS_DEFAULT_PORT;
    private long received = 0;
    private long outOfOrder = 0;
    private long nonBound = 0;
    private long timeOuts = 0;
    private long numSkipped = 0;
    private long oldsequence = 0;

    /**
     * Non-static running procedure. 
     * Called my main()
     * @param args Command line args. Rigth now they are ignored
     */
    public void run(String[] args) {


        jbp = new JBlinkenPanel();                      // Create the GUI
        jbp.addWindowListener(new WindowAdapter() {     // Add close listener
            
            @Override
            public void windowClosed(WindowEvent we) {
                System.out.println("Shutting down...");
                if (jbudp != null) {
                    jbudp.setToExit();
                    try {
                        jbudp.join(0);
                    } catch (InterruptedException ex) {
                    }
                }
                System.out.println("Shutdown complete.");
            }
        });

        jbp.startBT.addActionListener(new ActionListener() {    // START button

            @Override
            public void actionPerformed(ActionEvent ae) {
                if (jbudp != null) {
                    System.out.println("The UDP listener thread is alreay active.");
                } else {
                    System.out.println("Starting UDP listener thread...");
                    jbudp = new JbsUdpListener(currentPort);
                    jbudp.start();
                    System.out.println("UDP listener thread started.");
                    jbp.statusLabel.setText("Starting...");
                }
            }
        });
        jbp.stopBT.addActionListener(new ActionListener() {     // STOP button

            @Override
            public void actionPerformed(ActionEvent ae) {
                System.out.println("Stopping UDP listener thread...");
                jbp.statusLabel.setText("Stopping...");
                if (jbudp != null) {
                    try {
                        jbudp.setToExit();
                        jbudp.join(0);
                        jbp.statusLabel.setText("Stopped");
                        jbudp = null;
                    } catch (InterruptedException ex) {
                    }
                }
                System.out.println("UDP listener thread stopped.");
            }
        });

        jbp.zeroBT.addActionListener(new ActionListener() {     // Zero counters button

            @Override
            public void actionPerformed(ActionEvent ae) {
                received = 0;
                outOfOrder = 0;
                nonBound = 0;
                timeOuts = 0;
                numSkipped = 0;
            }
        });

        jbp.udpPortFTF.addActionListener(new ActionListener() { // UDP port change

            @Override
            public void actionPerformed(ActionEvent ae) {
                int newport = Integer.parseInt(jbp.udpPortFTF.getText());
                if (newport != currentPort) {
                    System.out.println("Switching from UDP port " + currentPort + " to " + newport);
                    System.out.flush();
                    jbudp.setToExit();
                    try {
                        jbp.statusLabel.setText("Stopping...");
                        jbudp.join(0);
                    } catch (InterruptedException ex) {
                    }
                    currentPort = newport;
                    jbudp = new JbsUdpListener(currentPort);
                    jbudp.start();
                    jbp.statusLabel.setText("Starting...");

                }
            }
        });

        java.awt.EventQueue.invokeLater(new Runnable() {    // Fire up the GUI

            @Override
            public void run() {
                jbp.setVisible(true);
            }
        });

        jbudp = new JbsUdpListener(currentPort);        // Create server thread
        jbudp.start();                                  // And go!
    }

    /**
     * Static main procedure
     * @param args the command line arguments (ignored)
     */
    public static void main(String[] args) {
        final JBlinkenServer jbs = new JBlinkenServer();
        jbs.run(args);
    }

    /**
     * Light error led
     * @param err turn on the LED if true, turn it off if false
     */
    private void setError(boolean err) {
        if (err) {
            jbp.errorLabel.setIcon(jbp.iconRed);
        } else {
            jbp.errorLabel.setIcon(jbp.iconBlack);
        }
    }

    /**
     * Light binary representation of a word
     * @param flags     Message flags (no parity, error, ...)
     * @param lights    2 bytes of binary data in big-endian order
     */
    private void setLights(int flags, int lights) {
        int n = lights;

        if ((flags & BLF_ERROR) != 0) {             // Error function?
            jbp.errorLabel.setIcon(jbp.iconRed);    // Yes, turn on red led
        } else {
            if ((flags & BLF_NOPARITY) == 0) {      // No inhibited parity
                if (parity(lights) == 1) {          // Compute and light parity
                    jbp.parityLabel.setIcon(jbp.iconOrange);
                } else {
                    jbp.parityLabel.setIcon(jbp.iconBlack);
                }
            } else {
                jbp.parityLabel.setIcon(jbp.iconBlack); // No parity
            }
            for (int i = 0; i < 16; i++) {          // Turn on leds for bits
                jbp.errorLabel.setIcon(jbp.iconBlack);
                if (n % 2 != 0) {
                    jbp.bitsLabel[i].setIcon(jbp.iconGreen);
                } else {
                    jbp.bitsLabel[i].setIcon(jbp.iconBlack);
                }
                n /= 2;
            }
        }
    }

    /**
     * Parity computation. It returns 1 if the number of "one" bits
     * is odd.
     * @param number input number
     * @return 1 if number of one bits in 'number' is odd, zero if even
     */
    private int parity(int number) {
        int nones = 0;
        int rem = number;

        while (rem > 0) {
            nones += rem % 2;
            rem /= 2;
        }

        return nones % 2;
    }

    /**
     * Inner private class. It implements the UDP listener and runs
     * in a separate thread.
     */
    private class JbsUdpListener extends Thread implements ActionListener {

        private boolean toExit = false;
        private int udpPort;
        private DatagramSocket socket = null;
        private int timeOut = BLS_DEFAULT_TIMEOUT;

        /**
         * Public constructor
         * @param port UDP port to listen on
         */
        public JbsUdpListener(int port) {
            super();
            this.udpPort = port;
        }

        /**
         * Signal to exit at the next loop iteration
         */
        public void setToExit() {
            toExit = true;
        }

        /**
         * Setter for the timeout attribute
         * @param timeOut Socked read timeout, in milliseconds
         */
        public void setTimeOut(int timeOut) {
            this.timeOut = timeOut;
        }

        /**
         * Open a datagram (UDP) socket
         * @return  Opened socket
         * @exception SocketException Exception creating the socket
         */
        private DatagramSocket openSocket() throws SocketException {
            DatagramSocket sock = null;
            sock = new DatagramSocket(udpPort);
            sock.setSoTimeout(timeOut);
            System.out.println("Listening on socket " + udpPort);
            return sock;
        }

        /**
         * Main thread procedure. It runs until the toExit boolean is set
         * to true
         */
        @Override
        public void run() {
            try {
                Timer timer = new Timer(1000, this);
                byte[] buffer = new byte[4096];
                int function = 0;
                int flags = 0;
                long sequence;
                int bflags = 0;
                int numDataBytes = 0;
                int numAddrBytes = 0;
                int numOtherBytes = 0;
                DataInputStream is = null;
                boolean usePacket = false;
                InetAddress pair = null;

                socket = openSocket();
                DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
                timer.start();          // Start stats update timer
                while (!toExit) {
                    try {
                        socket.receive(packet);
                        received++;
                        usePacket = false;
                        // Get the different fields from the datagram
                        // Java always uses network order, so no need to 
                        // mangle the bytes...
                        is = new DataInputStream(new ByteArrayInputStream(packet.getData()));
                        function = is.readShort();
                        flags = is.readShort();
                        sequence = is.readInt();
                        bflags = is.readShort();
                        numDataBytes = is.readUnsignedShort();
                        numAddrBytes = is.readUnsignedShort();
                        numOtherBytes = is.readUnsignedShort();
                        /*
                         * If we are already paired (bound) to a client, check
                         * if the packet comes from that host
                         */
                        if (pair != null) {
                            if (pair.equals(packet.getAddress())) {
                                if (sequence > oldsequence) {   // OK, use packet
                                    usePacket = true;       
                                } else if ((flags & FLG_RESYNC) != 0) {
                                    usePacket = true;           // Sequence not OK, but
                                } else {                        // resync requested
                                    outOfOrder++;               // No resync, packet out-of-order => ignored
                                    System.err.println("Packet out of sequence " + sequence + " vs " + oldsequence);
                                }
                            } else {                            // NOK, packet from non-bound source
                                System.err.println("Packet from non-bound source "
                                        + packet.getAddress() + " (bound address is " + pair + ")");
                                usePacket = false;
                                nonBound++;
                            }
                        } else {
                            pair = packet.getAddress();         // We are not yet paired, so we will do it now
                            jbp.statusLabel.setText("Running, bound to " + pair.getHostAddress());
                            usePacket = true;
                        }
                        if (usePacket) {
                            if (sequence - oldsequence > 1) {   // Check if we skipped packets
                                numSkipped += (sequence - oldsequence - 1);
                            }
                            oldsequence = sequence;

                            if (numDataBytes != 2) {            // Check packet payload
                                System.out.println("Wrong packet received, unsupported " + numDataBytes + " bytes packet.");
                            } else {    
                                int theNum = is.readUnsignedShort();
                                setLights(bflags, theNum);      // update the LEDs!
                            }
                        }
                    } catch (SocketTimeoutException ste) {      // Timeout. If we are bound, unbind
                        if (pair != null) {
                            System.out.println("Timeout reading packet, unbinding");
                            jbp.statusLabel.setText("Running, unbound");
                        }
                        pair = null;
                        oldsequence = 0;
                        timeOuts++;
                    } catch (IOException ex) {
                        System.err.println("IOError reading socket");
                        throw new RuntimeException(ex);
                    }
                }
                /*
                 * End of process: stop stats update timer
                 * and clean up
                 */
                timer.stop();
                timer = null;
                socket.close();
                socket = null;
            } catch (SocketException ex) {
                System.err.println("Error opening UDP socket on port " + udpPort);
                ex.printStackTrace(System.err);
                setToExit();
            }
        }

        /**
         * Update timer action. Update the GUI with the current value
         * of the counters.
         * @param ae  Event. Ignored.
         */
        @Override
        public void actionPerformed(ActionEvent ae) {
            NumberFormat nf = NumberFormat.getNumberInstance();
            jbp.recvdLabel.setText(nf.format(received));
            jbp.oofLabel.setText(nf.format(outOfOrder));
            jbp.nboundLabel.setText(nf.format(nonBound));
            jbp.nskipLabel.setText(nf.format(numSkipped));
            jbp.tmoutLabel.setText(nf.format(timeOuts));
        }
    };
}
