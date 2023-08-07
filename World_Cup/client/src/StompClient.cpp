#include "ConnectionHandler.h"
#include "StompProtocol.h"
#include <vector>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <thread>

using namespace std;
 void start(StompProtocol &stompProtocol,ConnectionHandler &handler){
		while(!stompProtocol.shouldterminate()){
			string ans="";
			if (handler.getLine(ans))
			{
				string output =stompProtocol.frameToOutput(ans);
				if(output != "")
					cout << output << endl;
			}
		}
	}

vector<string> split2(string s, string delimiter)
{
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != string::npos)
    {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start)); 
    return res;
}



int main(int argc, char *argv[]) {		
	while(1){
		bool loggedIn = false;
		ConnectionHandler connectionHandler("-1", -1);
		StompProtocol protocol;
		while(!loggedIn){
			string loginCommand = "";
			getline(cin, loginCommand);
			string host_port = split2(loginCommand, " ")[1];
			string host = split2(host_port, ":")[0];
			short port = boost::lexical_cast<short>(split2(host_port, ":")[1]);
			connectionHandler.host_ = host;
			connectionHandler.port_ = port;
			if (!connectionHandler.connect()) {
    	     	std::cerr << "Cannot connect to " << host << ":" << port << std::endl;
				continue;
			}
			vector<string> frameToSend = protocol.frameToSend(loginCommand);
			connectionHandler.sendLine(frameToSend.at(0));
			string ans = "";
			connectionHandler.getLine(ans);
			string output = protocol.frameToOutput(ans);
			cout << output << endl;
			if(output != "Login succesful"){
				connectionHandler.close();
				continue;
			}
			loggedIn = true;
		}
		
			thread reader(&start, ref(protocol), ref(connectionHandler));
		
		while(!protocol.shouldterminate()){
			string command;
			getline(cin, command);
			vector<string> frameToSend = protocol.frameToSend(command);		
			if(frameToSend.at(0) == "summary")
				continue;
			if(frameToSend.at(0) == "d")
				continue;
			unsigned int i =0;
			while(i<frameToSend.size()){
			connectionHandler.sendLine(frameToSend.at(i));
			i++;
		
		}
			if(command == "logout"){
				reader.join();
				connectionHandler.close();
			}
		}
	}
	return 0;
}