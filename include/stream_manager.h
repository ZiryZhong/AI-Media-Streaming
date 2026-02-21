#ifndef STREAM_MANAGER_H
#define STREAM_MANAGER_H

#include <string>
#include <map>
#include <memory>

class StreamSession;

class StreamManager {
private:
    std::map<std::string, std::shared_ptr<StreamSession>> sessions;

public:
    StreamManager();
    ~StreamManager();

    std::shared_ptr<StreamSession> createSession(const std::string& sessionId);
    std::shared_ptr<StreamSession> getSession(const std::string& sessionId);
    void removeSession(const std::string& sessionId);
};

#endif // STREAM_MANAGER_H
