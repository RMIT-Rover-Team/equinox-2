#include "comms/UnixWrapper.hpp"
#include "comms/GenericNetWrapper.hpp"
#include "comms/TCPWrapper.hpp"
#include "ChannelMulticast.hpp"
#include <filesystem>
#include <thread>
#include <chrono>

//Server Stuff
MulticastPublisher::MulticastPublisher(std::string defComFile){
    //Load the comms definitions
    this->definition = loadConfFile(defComFile);

    //Open the TCP server
    this->tcpServer = newTCPServer(this->definition.ResolvedIP, this->definition.NumericPort);

    //Open the Unix server
    this->unixServer = newUnixServer("/tmp/" + this->definition.Name);

    //Start the threads
    acceptorThreadTCP = std::thread(&MulticastPublisher::acceptorTCP, this);
    acceptorThreadUnix = std::thread(&MulticastPublisher::acceptorUnix, this);

}

MulticastPublisher::~MulticastPublisher(){
    //Join all the threads
    //acceptorThreadTCP.join();
    //acceptorThreadUnix.join();

    //Clean up dangling connections
    std::cout << "Shutting Down" << std::endl;
    for (size_t i = 0; i < clientConnections.size(); ) {
        delete clientConnections[i];
    }
    std::cout << "Destroy Clients" << std::endl;

}

void MulticastPublisher::acceptorTCP(){
    while(true){
        //Accept the connection
        TCPServerCon* client = new TCPServerCon(this->tcpServer);
        client->setPriority(this->definition.Priority);

        //Log the connection
        std::cout << "Accepted TCP connection" << std::endl;

        this->spawnClient(client);
    }
}

void MulticastPublisher::acceptorUnix(){
    while(true){
        //Accept the connection
        UnixServerCon* client = new UnixServerCon(this->unixServer);

        //Log the connection
        std::cout << "Accepted Unix connection" << std::endl;

        this->spawnClient(client);
    }
}

void MulticastPublisher::spawnClient(GenericNetWrapper* client){
    //Lock the acceptor mutex
    this->acceptMutex.lock();

    //Spawn the client
    clientConnections.push_back(client);
    clientThreads.emplace_back(&MulticastPublisher::clientProcess, this, client, idCounter);

    //Increment the client ID counter
    idCounter++;

    //Clean up dead threads
    for (size_t i = 0; i < clientConnections.size(); ) {
        if (!clientConnections[i]->report().Alive) {
            //Free the client
            delete clientConnections[i];

            //Join the thread
            clientThreads[i].join();

            //erase both at the same index
            clientConnections.erase(clientConnections.begin() + i);
            clientThreads.erase(clientThreads.begin() + i);

        } else {
            ++i;
        }
    }

    //Release the acceptor mutex
    this->acceptMutex.unlock();    
}

void MulticastPublisher::clientProcess(GenericNetWrapper* client, int clientID){
    //Create my entry in the send buffers
    this->sendMutex.lock();
    sendBuffers.emplace(clientID, std::vector<MessageStructure>{});
    this->sendMutex.unlock();

    while (client->report().Alive){
        while (sendBuffers[clientID].size() == 0){
            //Wait 0.1s
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (sendBuffers[clientID].size() > 0){
            this->sendMutex.lock();
            bool result = client->senddat(sendBuffers[clientID].back().getDecodeBuffer(), sendBuffers[clientID].back().totalSize);
            sendBuffers[clientID].pop_back();
            this->sendMutex.unlock();

            if (!result){
                break;
            }

        }

    }
    client->closeCon();
    std::cout << "Client closed" << std::endl;

    //Destroy my entry in the send buffers
    this->sendMutex.lock();
    this->sendBuffers.erase(clientID);
    this->sendMutex.unlock();

}


MessageStructure MulticastPublisher::getNewMessageObject(){
    //Create a new request object
    MessageStructure request(this->definition.RequestMessageFormat);

    return request;
}


void MulticastPublisher::publish(MessageStructure requestData){
    //Lock the mutex
    this->sendMutex.lock();

    //Add the message to each of the send buffers
    for (auto& clientID : sendBuffers){
        clientID.second.emplace_back(requestData);
    }
    
    //Unlock the mutex
    this->sendMutex.unlock();

}


// Client Stuff
MulticastSubscriber::MulticastSubscriber(std::string defComFile){
    //Load the comms definitions
    this->definition = loadConfFile(defComFile);

    //Check if the unix file exists first
    if (std::filesystem::exists("/tmp/" + this->definition.Name)) {
        this->con = new UnixClientCon("/tmp/" + this->definition.Name);
        std::cout << "Connected Unix to " << this->definition.Name << std::endl;
    }
    else {
        this->con = new TCPClientCon(this->definition.ResolvedIP, this->definition.NumericPort);
        this->con->setPriority(this->definition.Priority);
        std::cout << "Connected TCP to " << this->definition.ResolvedIP << ":" << this->definition.NumericPort << std::endl;
    }

}


MulticastSubscriber::~MulticastSubscriber(){
    //Close the connection
    this->con->closeCon();
    delete this->con;
}



MessageStructure MulticastSubscriber::subscribe(){

    // Get the response
    unsigned char* message = this->con->getdat(this->definition.RequestMessageFormat.totalSize);

    //Decode the response
    MessageStructure response(this->definition.RequestMessageFormat);
    if (message != nullptr){
        response.setDecodeBuffer(message);
    }

    return response;
}