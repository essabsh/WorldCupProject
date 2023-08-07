package bgu.spl.net.srv;

import bgu.spl.net.impl.stomp.user;

import java.io.IOException;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

public class ConnectionsImpl<T> implements Connections<T>{

    public ConcurrentHashMap<Integer, ConnectionHandler> idToHandler;
    public ConcurrentHashMap<String, user> nametouser;
    public ConcurrentHashMap<String, LinkedList<user>> topictousers;
    public ConcurrentHashMap<Integer,user> conIDtouser;
    public int messageID=0;
    private static ConnectionsImpl instance = null;

    private ConnectionsImpl(){
        idToHandler = new ConcurrentHashMap<>();
        nametouser =new ConcurrentHashMap<>();
        topictousers=new ConcurrentHashMap<>();
        conIDtouser=new ConcurrentHashMap<>();
    }

    public static synchronized ConnectionsImpl getInstance() {
        if (instance == null) {
            instance = new ConnectionsImpl();
        }
        return instance;
    }
    public String preparemsg(String channel,String msg,user user1){
        String output="MESSAGE\nsubscription:"+user1.topictosubID.get(channel)+"\nmessage-id"+(messageID++)+"\n"+"destenation:"+channel+"\n\n"+msg;
        return output;
    }

    public boolean send(int connectionId, String msg){
        if(idToHandler.containsKey(connectionId)) {
            idToHandler.get(connectionId).send(msg);
            return true;
        }
        return false;
    }

    public void send(String channel, String msg){
        if (topictousers.isEmpty())
            return;
        LinkedList<user> subs=topictousers.get(channel);
        for (int i=0;i<subs.size();i++){
            boolean f=send(subs.get(i).conID,preparemsg(channel,msg,subs.get(i)));
            System.out.println("send to "+subs.get(i).conID);
            System.out.println(msg);
            if (!f)
                prepareerror(channel,msg,subs.get(i));
        }
    }

    private void prepareerror(String channel, String msg, user user) {
        String output="ERROR\nreceipt-id:message-"+messageID+"\nmessage:malformed frame received\n\n-----\nSEND\ndestined:"
                +channel+"+receipt:message-"+messageID+"\n\n"+msg;
        send(user.conID,output);
    }

    @Override
    public boolean send(int connectionId, T msg) {
        return false;
    }

    @Override
    public void send(String channel, T msg) {

    }

    public void disconnect(int connectionId){
       idToHandler.remove(connectionId);
       user user1=conIDtouser.get(connectionId);
       user1.loggedin=false;
       conIDtouser.remove(connectionId);
       Set<String> keys=topictousers.keySet();
       for (String s:keys){
           if (topictousers.get(s).contains(user1))
               topictousers.get(s).remove(user1);
       }
    }

    public void addConnection(int connectionId, ConnectionHandler handler){
        idToHandler.put(connectionId, handler);
    }
    public boolean addToUser(String username,String passcode,int conID){
        if (!nametouser.containsKey(username)){
            user user1=new user(username,passcode,conID);
            nametouser.put(username,user1);
            conIDtouser.put(conID,user1);
            return true;
        }
        return false;
    }
    public void changeconID(String username,int conID){
        user user1=nametouser.get(username);
        user1.conID=conID;
        conIDtouser.put(conID,user1);
    }
    //subscribe
    public void addtotopic(int conID,String topic,int subID) {
        user user1=conIDtouser.get(conID);
        user1.topictosubID.put(topic,subID);
        if (!topictousers.containsKey(topic))
            topictousers.put(topic, new LinkedList<user>());
        topictousers.get(topic).add(user1);
        System.out.println(topictousers.get(topic).size());
        user1.topictosubID.put(topic,subID);
    }
    //unsubscribe
    public void removefromtopic(int conID,int subID){
        user user1=conIDtouser.get(conID);
        String topic="";
        Set<String> keys=user1.topictosubID.keySet();
        for (String s : keys){
            if (user1.topictosubID.get(s)==subID)
                topic=s;
        }
        user1.topictosubID.remove(topic);
        topictousers.get(topic).remove(user1);
    }
}
