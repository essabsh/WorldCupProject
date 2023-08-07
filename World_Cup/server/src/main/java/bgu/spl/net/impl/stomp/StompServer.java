package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.Server;

public class StompServer {

    public static void main(String[] args) {
        // TODO: implement this
        int port=Integer.parseInt(args[0]);
        if (args[1].equals("tpc")) {
            Server.threadPerClient(
                    port, //port
                    () -> new StompProtocol(), //protocol factory
                    () -> new StompEncoderDecoder()//message encoder decoder factory
            ).serve();
        }
        if (args[1].equals("reactor")){
            Server.reactor(
                    4,
                    port, //port
                    () -> new StompProtocol(), //protocol factory
                    () -> new StompEncoderDecoder()//message encoder decoder factory
            ).serve();
        }
    }
}
