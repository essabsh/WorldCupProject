#include "StompProtocol.h"
#include <atomic>
#include <iostream>
#include <fstream>
#include  <map>

std::atomic<int> receipt(0);
std::atomic<int> subscribe(0);

vector<string> split(string s, string delimiter)
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

string maptoString(map<string, string> map)
{
    string output = "";
    for (std::map<string, string>::iterator iter = map.begin(); iter != map.end(); ++iter)
    {
        string key = iter->first;
        string value = iter->second;
        output = output + "    " + key + ":" + value + "\n";
    }
    return output;
}
string EventtoString(Event eve)
{
    string output = "";
    string team_a = eve.get_team_a_name();
    string team_b = eve.get_team_b_name();
    string event_name = eve.get_name();
    int time = eve.get_time();
    map<string, string> mapgeneralupdates = eve.get_game_updates();
    string generalupdates = maptoString(mapgeneralupdates);
    map<string, string> mapteam_a_updates = eve.get_team_a_updates();
    string team_a_updates = maptoString(mapteam_a_updates);
    map<string, string> mapteam_b_updates = eve.get_team_b_updates();
    string team_b_updates = maptoString(mapteam_b_updates);
    string description = eve.get_discription();
    output = "team a:" + team_a + "\n" + "team b:" + team_b + "\n" + "event name:" + event_name + "\n" + "time:" + std::to_string(time) + "\n" + "general game updates:" + "\n" + generalupdates + "team a updates:" + "\n" + team_a_updates + "team b updates:" + "\n" + team_b_updates + "description:" + description + "\n \n";
    return output;
}

vector<string> StompProtocol::frameToSend(string command)
{
    vector<string> commandToWords = split(command, " ");
    vector<string> frame;
    if (commandToWords.at(0) == "login")
    {
        if (!loggedin)
        {
            logout = false;
            string username = commandToWords.at(2);
            string passcode = commandToWords.at(3);
            user = username;
            int recieptid = receipt.fetch_add(1);
            frame.push_back("CONNECT\naccept-version:1.2\nhost:stomp.cs.bgu.ac.il\nlogin:" + username + "\npasscode:" + passcode + "\nreciept-id:" + std::to_string(recieptid) + "\n \n");
            recieptTosentframe.insert(pair<int, string>(recieptid, frame.at(0)));
        }
        else
        {
            cout << "The client is already logged in, log out before trying again" << endl;
            frame.push_back("d");
        }
    }
    if (commandToWords.at(0) == "logout")
    {
        int recieptid = receipt.fetch_add(1);
        frame.push_back("DISCONNECT\nreceipt:" + std::to_string(recieptid) + "\n \n");
        terminate = true;
        loggedin = false;
        logout = true;
    }
    if (commandToWords.at(0) == "join")
    {
        string destination = commandToWords.at(1);
        int subscribeid = subscribe.fetch_add(1);
        int recieptid = receipt.fetch_add(1);
        frame.push_back("SUBSCRIBE\ndestination:/" + destination + "\nid:" + std::to_string(subscribeid) + "\nreceipt:" + std::to_string(recieptid) + "\n \n");
        TopicTosubid.insert(pair<string, int>(destination, subscribeid));
        recieptTosentframe.insert(pair<int, string>(recieptid, frame.at(0))); 
        recieptToDest.insert(pair<int, string>(recieptid, destination));
    }
    if (commandToWords.at(0) == "exit")
    {
        string destination = commandToWords.at(1);
        int id = TopicTosubid[destination];
        TopicTosubid.erase(destination);
        int recieptid = receipt.fetch_add(1);
        frame.push_back("UNSUBSCRIBE\nid:" + std::to_string(id) + "\nreceipt:" + std::to_string(recieptid) + "\n \n");
        recieptTosentframe.insert(pair<int, string>(recieptid, frame.at(0)));
    }
    if (commandToWords.at(0) == "report")
    {
        string file = commandToWords.at(1);
        names_and_events curr = parseEventsFile(file);
        string team_a = curr.team_a_name;
        string team_b = curr.team_b_name;
        string dest = team_a + "_" + team_b;
        vector<Event> evs = curr.events;
        unsigned int i = 0;
        if(TopicTosubid.count(dest)>0){
        while (i < evs.size())
        {
            Event curr_ev = evs[i];
            frame.push_back("SEND\ndestination:/" + dest + "\n \n" + "user:" + user + "\n" + EventtoString(curr_ev) + "\n \n");
            i++;
        }
    }
    else {
        vector<string> s;
        s.push_back("d");
        return s;
    }
    }
    if (commandToWords.at(0) == "summary")
    {
        string game = commandToWords.at(1);
        if(!(TopicTosubid.count(game)>0)){
            vector<string> s;
            s.push_back("d");
            return s;
        }
        string team_a = split(game, "_").at(0);
        string team_b = split(game, "_").at(1);
        frame.push_back(team_a + " vs " + team_b + "\nGame stats:\nGeneral stats:\n");
        string sender = commandToWords.at(2);
        string file = commandToWords.at(3);
        vector<Event> evs = UsertoGameEvents[sender][game];
        unsigned int i = 0;
        map <string,string> game_stats;
        while (i < evs.size())
        {
            Event curr_ev = evs[i];
            map<string, string> mapgameupdates = curr_ev.get_game_updates();
            for(auto &pair: mapgameupdates)
                game_stats[pair.first] = pair.second;
            i++;
        }
        string gameUpdates = maptoString(game_stats);
        frame.at(0) = frame.at(0) + gameUpdates + team_a + " stats:\n";
        unsigned int j = 0;
        map <string,string> a_stats;
        map <string,string> b_stats;

        while (j < evs.size())
        {
            Event curr_ev = evs[j];
            map<string, string> mapteam_a_updates = curr_ev.get_team_a_updates();
            for(auto &pair: mapteam_a_updates)
                a_stats[pair.first] = pair.second;
            j++;
        }
        string team_a_updates = maptoString(a_stats);
        frame.at(0) = frame.at(0) + team_a_updates  + team_b + " stats:\n";
        unsigned int k = 0;
        while (k < evs.size())
        {
            Event curr_ev = evs[k];
            map<string, string> mapteam_b_updates = curr_ev.get_team_b_updates();
            for(auto &pair: mapteam_b_updates)
                b_stats[pair.first] = pair.second;
            k++;
        }
        string team_b_updates = maptoString(b_stats);
        frame.at(0) = frame.at(0) + team_b_updates;

        frame.at(0) = frame.at(0) + "Game event reports:";
        unsigned int w = 0;
        while (w < evs.size())
        {
            Event curr_ev = evs[w];
            int curr_time = curr_ev.get_time();
            string name = curr_ev.get_name();
            string description = curr_ev.get_discription();
            frame.at(0) = frame.at(0) + "\n"+std::to_string(curr_time) + " - " + name + ":" + "\n \n" + description + "\n\n";
            w++;
        }
        ofstream f(file);
        f << frame.at(0);
        f.close();
        vector<string> summary;
        summary.push_back("summary");
        return summary;
    }
    return frame;
}

string StompProtocol::frameToOutput(string frame)
{
    vector<string> frameToLines = split(frame, "\n");
    if (frameToLines[0] == "CONNECTED")
    {
        loggedin = true;
        return "Login succesful";
    }
    if (frameToLines[0] == "RECEIPT")
    {
        int recent = stoi(frameToLines.at(1).substr(11));
        vector<string> sentframeToLines = split(recieptTosentframe[recent], "\n");
        if (sentframeToLines[0] == "SUBSCRIBE")
        {
            return "Joined channel " + recieptToDest[recent];
        }
        else if (sentframeToLines.at(0) == "UNSUBSCRIBE")
            return "Exited channel " + recieptToDest[recent];
        else{
            logout = true;
            return "disconnected.";
        }
    }
    if (frameToLines[0] == "MESSAGE")
    {

        string team_a=frameToLines[6].substr(7);
        string team_b=frameToLines[7].substr(7);
        string game = team_a+"_"+team_b;
        string event_name = frameToLines[8].substr(11);
        int time = stoi(frameToLines[9].substr(5));
        map<string,string> general_game_updates;
        int i=11;
        while (frameToLines[i]!="team a updates:"){
            vector<string> updateparts = split(frameToLines[i], ":");
            string key  = updateparts[0];
            string value = updateparts[1];
            general_game_updates[key] = value; 
            i++;
        }
        i++;
        map<string,string> team_a_updates;
        while(frameToLines[i]!="team b updates:"){
            vector<string> updateparts = split(frameToLines[i], ":");
            string key  = updateparts[0];
            string value = updateparts[1];
            team_a_updates[key] = value; 
            i++;
        } 
        i++;
        map<string,string> team_b_updates;
        while(split(frameToLines[i], ":").at(0) !="description"){
            vector<string> updateparts = split(frameToLines[i], ":");
            string key  = updateparts[0];
            string value = updateparts[1];
            team_b_updates[key] = value; 
            i++;
        }
        string description = frameToLines[i].substr(12);
        string user1 = frameToLines[5].substr(5);
        Event eve(team_a,team_b,event_name,time,general_game_updates,team_a_updates,team_b_updates,description);
        UsertoGameEvents[user1][game].push_back(eve);
        return "";
    }
    if (frameToLines[0] == "ERROR")
    {
        string wrongpass = "wrong passcode";
        string alreadyloggedin = "user already logged in";
        if (frameToLines.at(2).substr(8) == wrongpass)
            return wrongpass;
        if (frameToLines.at(2).substr(8) == alreadyloggedin)
            return alreadyloggedin;
    }
}
bool StompProtocol::shouldterminate(){
    return terminate;
}


