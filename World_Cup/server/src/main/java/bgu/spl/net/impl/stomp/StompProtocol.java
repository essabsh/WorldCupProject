package bgu.spl.net.impl.stomp;

import java.util.*;
import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.ConnectionsImpl;
import java.util.concurrent.ConcurrentHashMap;
import bgu.spl.net.impl.stomp.user;

public class StompProtocol implements StompMessagingProtocol<String> {
    boolean terminate;
    public ConnectionsImpl connections;
    private int connID;

    public StompProtocol(){

    }
    public void start(int connectionId, Connections<String> connections){
        this.connections = (ConnectionsImpl) connections;
        connID = connectionId;
        terminate=false;
    }

    public void process(String message){
        //response
        String s=message.substring(0,4);
        if (s.equals("SEND")){
            String[] frametosend=message.split("\n \n");
            String msg=frametosend[1];
            String[] header=frametosend[0].split("\n");
            String channel=header[1].substring(13);
            connections.send(channel,msg);
        }
        String []frameToLines = message.split("\n");
        String response = "";
        //frame to send.
        switch (frameToLines[0]){
            case "CONNECT":
                String username=frameToLines[3].substring(6);
                String passcode=frameToLines[4].substring(9);


                boolean notexist=connections.addToUser(username,passcode,connID);
                if (notexist) {
                    response = "CONNECTED\nversion:1.2\n \n";
                    connections.send(connID, response);
                }
                else {
                    user a=(user) connections.nametouser.get(username);
                    if ((a.passcode.equals(passcode)) ) {
                        if (!a.loggedin) {
                            connections.changeconID(username,connID);
                            response = "CONNECTED\nversion:1.2\n \n";
                            connections.send(connID, response);
                        }
                        //to complete errors
                        else {
                            response = "ERROR\nrecipet-id:"+frameToLines[5].substring(11)+"\nmessage:user already logged in\n \n";
                            connections.send(connID, response);
                        }
                    }
                    if (!a.passcode.equals(passcode)) {
                        //to complete errors
                        response = "ERROR\nrecipet-id:"+frameToLines[5].substring(11)+"\nmessage:wrong passcode\n \n";
                        System.out.println(response);
                        connections.send(connID, response);
                    }
                }
                break;
            case "SUBSCRIBE":
                //to complete
                response="RECEIPT\nreceipt-id:"+frameToLines[3].substring(8)+"\n \n";
                String id=frameToLines[2].substring(3);
                int x=Integer.parseInt(id);
                connections.addtotopic(connID,frameToLines[1].substring(13),x);
                connections.send(connID,response);
                break;
            case "UNSUBSCRIBE":
                response="RECEIPT\nreceipt-id:"+frameToLines[2].substring(8)+"\n \n";
                connections.removefromtopic(connID,Integer.parseInt(frameToLines[1].substring(3)));
                connections.send(connID,response);
                break;
            case "DISCONNECT":
                response="RECEIPT\nreceipt-id:"+frameToLines[1].substring(8)+"\n \n";
                user u = (user)connections.conIDtouser.get(connID);
                connections.send(connID,response);
                u.disconnect();
                connections.disconnect(connID);
                break;
        }
    }

    /**
     * @return true if the connection should be terminated
     */
    public boolean shouldTerminate(){
        return terminate;
    }
}
