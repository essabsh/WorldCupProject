package bgu.spl.net.impl.stomp;

import java.util.concurrent.ConcurrentHashMap;

public class user {
    public String username;
    public String passcode;
    public boolean loggedin;
    public int conID;
    public ConcurrentHashMap<String,Integer> topictosubID;

    public user(String username,String passcode,int conID){
        this.username=username;
        this.passcode=passcode;
        this.loggedin=true;
        this.conID=conID;
        topictosubID=new ConcurrentHashMap<>();
    }

    public user() {

    }

    public void disconnect(){
        loggedin=false;
        topictosubID=new ConcurrentHashMap<>();
    }

    public String getPasscode() {
        return passcode;
    }
}
