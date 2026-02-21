#include "stream_manager.h"
#include "stream_session.h"
#include "logger.h"

StreamManager::StreamManager() {
    LOG_INFO("StreamManager initialized");
}

StreamManager::~StreamManager() {
    LOG_INFO("StreamManager destroyed");
    sessions.clear();
}

std::shared_ptr<StreamSession> StreamManager::createSession(const std::string& sessionId) {
    auto session = std::make_shared<StreamSession>(sessionId);
    sessions[sessionId] = session;
    LOG_INFO("Created session: " + sessionId);
    return session;
}

std::shared_ptr<StreamSession> StreamManager::getSession(const std::string& sessionId) {
    auto it = sessions.find(sessionId);
    if (it != sessions.end()) {
        LOG_DEBUG("Retrieved session: " + sessionId);
        return it->second;
    }
    LOG_WARNING("Session not found: " + sessionId);
    return nullptr;
}

void StreamManager::removeSession(const std::string& sessionId) {
    sessions.erase(sessionId);
    LOG_INFO("Removed session: " + sessionId);
}
