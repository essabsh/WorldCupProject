#pragma once

#include "../include/ConnectionHandler.h"
#include "../include/event.h"
#include <atomic>
#include <map>
using namespace std;

// TODO: implement the STOMP protocol
class StompProtocol
{
private:
public:
    map<string,int> TopicTosubid; 
    map<int,string> recieptTosentframe;
    map<int,string> recieptToDest;
    map<string,vector<Event>> gameToEvents;
    map<string,map<string,vector<Event>>> UsertoGameEvents;
    string user;
    bool loggedin;
    bool logout;
    bool terminate;
    vector<string> frameToSend(string command);
    string frameToOutput(string frame);
    bool shouldterminate();
};
