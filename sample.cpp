void NonBlockingServer::run()
{
    while (true)
    {

        int nev = eventManager->eventListener();

        for (int i = 0; i < nev; i++) {

            std::cout << "At all?\n";

            EventBlock eventBlock = eventManager->getEvent(i);

            if (eventBlock.isRead && (eventBlock.fd == server_socket)) {
                acceptNewClient();
            }
            else if (eventBlock.isRead && (eventBlock.fd != server_socket)) {
                handleClientRead(eventBlock.fd);
            }
            else if (eventBlock.isWrite) {
                handleClientWrite(eventBlock.fd);
            }
        }
        removeDisconnectedClients();
    }
}
