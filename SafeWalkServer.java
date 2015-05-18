/**
   * Project 5
   * @author Adam Baker, baker323, Lab 813
   * @author Pedro Del Moral Lopez, pdelmora, Lab 810
   */

import java.io.*;
import java.net.*;
import java.util.Arrays;
import java.util.ArrayList;
import java.lang.NumberFormatException;

public class SafeWalkServer implements Runnable {
    public static void main(String[] args) throws IOException, SocketException, ClassNotFoundException {
        if (args.length == 0) {
            SafeWalkServer safeWalk = new SafeWalkServer();
            safeWalk.run();
        }
        if (args.length == 1) {
            int port = 0;
            try {
                port = Integer.parseInt(args[0]);
            } catch (NumberFormatException e) {
            }
            if (port > 65535 || port < 1025)
                System.out.println("Invalid port number. Exiting gracefully.");
            else {
                SafeWalkServer safeWalk = new SafeWalkServer(port);
                safeWalk.run();
            }
        }
    }
    
    /**
     * Construct the server, set up the socket.
     * 
     * @throws SocketException if the socket or port cannot be obtained properly.
     * @throws IOException if the port cannot be reused.
     */
    ServerSocket serverSocket;
    public SafeWalkServer(int port) throws IOException {
        ServerSocket serverSocket = new ServerSocket(port);
        this.serverSocket = serverSocket;
        System.out.printf("Constructing server at port %d\n", getLocalPort());
        serverSocket.setReuseAddress(true);
    }
    
    /**
     * Construct the server and let the system allocate it a port.
     * 
     * @throws SocketException if the socket or port cannot be obtained properly.
     * @throws IOException if the port cannot be reused.
     */
    public SafeWalkServer() throws IOException {
        ServerSocket serverSocket = new ServerSocket(0);
        this.serverSocket = serverSocket;
        System.out.printf("Port not specified. Using free port %d.\n", getLocalPort());
        serverSocket.setReuseAddress(true);
    }
    
    /**
     * Return the port number on which the server is listening. 
     */
    public int getLocalPort() {
        return serverSocket.getLocalPort();
    }
    
    /**
     * Start a loop to accept incoming connections.
     */
    public void run() {
        try {
            ArrayList<String[]> waiting = new ArrayList<String[]>();
            ArrayList<Socket> socketList = new ArrayList<Socket>();
            ArrayList<String> requests = new ArrayList<String>();
            while (true) {
                Socket client = serverSocket.accept();
                socketList.add(client);
                System.out.printf("connection received from %s%n", client);
                PrintWriter pw = new PrintWriter(client.getOutputStream());
                pw.flush();
                BufferedReader in = new BufferedReader(new InputStreamReader(client.getInputStream()));
                String str = (String) in.readLine();
                requests.add(str);
                if (str.charAt(0) == ':') {
                    if (str.equals(":LIST_PENDING_REQUESTS")) {
                        socketList.remove(socketList.size() - 1);
                        requests.remove(requests.size() - 1);
                        if (waiting.size() == 0) {
                            pw.println("No pending requests.");
                            pw.flush();
                            client.close();
                        }
                        // write to the client all unpaired request messages
                        // write to the client our ArrayList waiting list
                        else {
                            String[][] array = waiting.toArray(new String[waiting.size()][waiting.size()]);
                            String[] nums = new String[array.length];
                            for (int i = 0; i < array.length; i++) {
                                nums[i] = Arrays.toString(array[i]);
                            }
                            String message = Arrays.toString(nums);
                            pw.println(message);
                            pw.flush();
                            client.close();
                        }
                        // in the format [[Cyndi, EE, *, 0], [John Doe, PUSH, PMU, 0]]\n
                    }
                    // for each waiting request message, respond with ERROR: connection reset\n
                    // and close its socket before discarding the request
                    // respond to client with RESPONSE: success\n and close its socket
                    else if (str.equals(":RESET")) {
                        socketList.remove(socketList.size() - 1);
                        requests.remove(requests.size() - 1);
                        String[][] array = waiting.toArray(new String[waiting.size()][waiting.size()]); // iterating through the waiting list
                        int size = array.length;
                        for (int i = 0; i < size; i++) {
                            try {
                                PrintWriter print = new PrintWriter(socketList.get(i).getOutputStream());
                                print.flush();
                                BufferedReader buff = new BufferedReader(new InputStreamReader(socketList.get(i).getInputStream()));
                                print.println("ERROR: connection reset");
                                print.flush();
                                waiting.clear();
                                socketList.get(i).close(); // throws an exception
                            }
                            catch (Exception e) {
                                socketList.get(i).close();
                            }
                        }
                        pw.println("RESPONSE: success");
                        pw.flush();
                        client.close();
                    }
                    // gracefully terminate the server, close the socket and whatever streams your server uses
                    // and exit the run loop
                    else if (str.equals(":SHUTDOWN")) {
                        String[][] array = waiting.toArray(new String[waiting.size()][waiting.size()]); // iterating through the waiting list
                        int size = array.length;
                        for (int i = 0; i < size; i++) {
                            try {
                                PrintWriter print = new PrintWriter(socketList.get(i).getOutputStream());
                                print.flush();
                                BufferedReader buff = new BufferedReader(new InputStreamReader(socketList.get(i).getInputStream()));
                                print.println("ERROR: connection reset");
                                print.flush();
                                waiting.clear();
                                socketList.get(i).close(); // throws an exception
                            }
                            catch (Exception e) {
                                socketList.get(i).close();
                            }
                        }
                        pw.println("RESPONSE: success");
                        pw.flush();
                        serverSocket.close();
                        client.close();
                        pw.close();
                        in.close();
                        break;
                    }
                    else {
                        socketList.remove(socketList.size() - 1);
                        requests.remove(requests.size() - 1);
                        pw.println("ERROR: invalid request");
                        pw.flush();
                        client.close();
                        // respond with ERROR: invalid request\n and close its socket
                    }
                }
                if (str.charAt(0) != ':') {
                    String[] parts = str.split(",");
                    try {
                        String[] token = {parts[0], parts[1], parts[2], parts[3]};
                        if (parts.length != 4) { // must contain four tokens
                            socketList.remove(socketList.size() - 1);
                            requests.remove(requests.size() - 1);
                            pw.println("ERROR: invalid request");
                            pw.flush();
                            client.close();
                        }
                        else if (parts[1].equals("*")) { // FROM cannot be *
                            socketList.remove(socketList.size() - 1);
                            requests.remove(requests.size() - 1);
                            pw.println("ERROR: invalid request");
                            pw.flush();
                            client.close();
                        }
                        else if (parts[1].equals(parts[2])) { // FROM and TO cannot be the same
                            socketList.remove(socketList.size() - 1);
                            requests.remove(requests.size() - 1);
                            pw.println("ERROR: invalid request");
                            pw.flush();
                            client.close();
                        }
                        else if (!parts[1].equals("LWSN") && !parts[1].equals("CL50") && !parts[1].equals("EE") &&
                                 !parts[1].equals("PMU") && !parts[1].equals("PUSH")) { // check if FROM location is known
                            socketList.remove(socketList.size() - 1);
                            requests.remove(requests.size() - 1);
                            pw.println("ERROR: invalid request");
                            pw.flush();
                            client.close();
                        }
                        else if (!parts[2].equals("LWSN") && !parts[2].equals("CL50") && !parts[2].equals("EE") &&
                                 !parts[2].equals("PMU") && !parts[2].equals("PUSH") && !parts[2].equals("*")) {
                            // check if TO location is known
                            socketList.remove(socketList.size() - 1);
                            requests.remove(requests.size() - 1);
                            pw.println("ERROR: invalid request");
                            pw.flush();
                            client.close();
                        }
                        else {
                            System.out.println(Arrays.toString(token));
                            if (waiting.size() == 0) {
                                waiting.add(token);
                            }
                            // try to find a client with the same TO and FROM or if one client has its TO being *
                            else if (waiting.size() > 0) {
                                String[][] array = waiting.toArray(new String[waiting.size()][waiting.size()]);
                                ArrayList<Integer> index = new ArrayList<Integer>();
                                int size = array.length;
                                System.out.println(waiting.size());
                                System.out.println(size);
                                for (int i = 0; i < size; i++) {
                                    try {
                                        String[] field = waiting.get(i);
                                        // if TO and FROM are the same and both are not volunteers
                                        if (((parts[1].equals(field[1]) && parts[2].equals(field[2])) &&
                                             (!parts[2].equals("*") && (!field[2].equals("*")))) || 
                                            ((parts[1].equals(field[1]) && // if FROM are the same and one is a volunteer
                                              ((!field[2].equals("*") && parts[2].equals("*")) || 
                                               (field[2].equals("*") && !parts[2].equals("*")))))) {
                                            index.add(i);
                                        }
                                        else {
                                            if (!waiting.contains(token)) {
                                                waiting.add(token);
                                            }
                                        }
                                    } catch (IndexOutOfBoundsException e) {
                                    }
                                }
                                if (index.size() > 0) {
                                    System.out.println(index.get(0));
                                    if (socketList.size() > 3)
                                        System.out.println(socketList.get(3));
                                    if (socketList.size() > 2)
                                        System.out.println(socketList.get(2));
                                    System.out.println(socketList.get(1));
                                    System.out.println(socketList.get(index.get(0)));
                                    int newIndex = index.get(0);
                                    if (socketList.get(newIndex).isClosed()) {
                                        while (socketList.get(newIndex).isClosed()) {
                                            newIndex = newIndex + 1;
                                            System.out.println(socketList.get(newIndex));
                                        }
                                    }
                                    pw.println("RESPONSE: " + requests.get(newIndex));
                                    pw.flush();
                                    PrintWriter print = new PrintWriter(socketList.get(newIndex).getOutputStream());
                                    print.flush();
                                    BufferedReader buff = new BufferedReader(new InputStreamReader(socketList.get(newIndex).getInputStream()));
                                    print.println("RESPONSE: " + str);
                                    print.flush();
                                    socketList.get(newIndex).close();
                                    client.close();
                                    waiting.remove(newIndex);
                                    waiting.remove(token);
                                    System.out.println(socketList.get(socketList.size() - 1));
                                    socketList.remove(socketList.size() - 1);
                                    System.out.println(socketList.get(newIndex));
                                    socketList.remove(newIndex);
                                    System.out.println(requests.get(newIndex));
                                    requests.remove(newIndex);
                                    System.out.println(requests.get(requests.indexOf(str)));
                                    requests.remove(str);
                                    // respond to each of the pair their match's request message prefixed with RESPONSE:
                                    // close their sockets
                                }
                            }
                        }
                        
                    } catch (ArrayIndexOutOfBoundsException e) {
                        if (parts.length != 4) { // must contain four tokens
                            socketList.remove(socketList.size() - 1);
                            requests.remove(requests.size() - 1);
                            pw.println("ERROR: invalid request");
                            pw.flush();
                            client.close();
                        }
                    }
                }
            }
        } catch (Exception e) {
        }
    }
}