/// \file Application.cc
/*
 *
 * Application.cc source template automatically generated by a class generator
 * Creation date : ven. sept. 5 2014
 *
 * This file is part of DQM4HEP libraries.
 * 
 * DQM4HEP is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * based upon these libraries are permitted. Any copy of these libraries
 * must include this copyright notice.
 * 
 * DQM4HEP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with DQM4HEP.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * @author Remi Ete
 * @copyright CNRS , IPNL
 */

// -- dqm4hep headers
#include "dqm4hep/Application.h"
#include "dqm4hep/OnlineRoutes.h"
#include "dqm4hep/Logging.h"

namespace dqm4hep {

  namespace online {
    
    const std::string &Application::type() const {
      return m_type;
    }
    
    //-------------------------------------------------------------------------------------------------
    
    void Application::setType(const std::string &type) {
      if(initialized()) {
        dqm_error( "Application::setType(): Couldn't set app type, app is already initialized !" );
        throw core::StatusCodeException(core::STATUS_CODE_NOT_ALLOWED);
      }
      m_type = type;
    }
    
    //-------------------------------------------------------------------------------------------------

    const std::string &Application::name() const {
      return m_name;
    }
    
    //-------------------------------------------------------------------------------------------------

    void Application::setName(const std::string &name) {
      if(initialized()) {
        dqm_error( "Application::setName(): Couldn't set app name, app is already initialized !" );
        throw core::StatusCodeException(core::STATUS_CODE_NOT_ALLOWED);
      }
      m_name = name;
    }
    
    //-------------------------------------------------------------------------------------------------
    
    const std::string &Application::state() const {
      return m_state;
    }
    
    //-------------------------------------------------------------------------------------------------
    
    void Application::setState(const std::string &state) {
      m_state = state;
      
      if(m_pAppStateService && m_server && m_server->isRunning())
        m_pAppStateService->send(m_state);
    }
    
    //-------------------------------------------------------------------------------------------------

    void Application::createStatsEntry(const std::string &name, const std::string &unit, const std::string &description) {
      if(initialized()) {
        dqm_error( "Application::createStatsEntry(): Couldn't create stat entry, app is already initialized !" );
        throw core::StatusCodeException(core::STATUS_CODE_NOT_ALLOWED);
      }
      
      if(not m_statistics[name].is_null())
        throw core::StatusCodeException(core::STATUS_CODE_ALREADY_PRESENT);
      
      // add stat entry
      m_statistics[name] = {
        {"description", description},
        {"unit", unit}
      };
    }
  
    //-------------------------------------------------------------------------------------------------

    void Application::sendStat(const std::string &name, double stats) {
      if(not statsEnabled()) {
        return;
      }
      core::json object = m_statistics[name];
      if(object.is_null())
        throw core::StatusCodeException(core::STATUS_CODE_NOT_FOUND);
      // add metadata on the fly
      object["name"] = name;
      object["value"] = stats;
      object["appType"] = this->type();
      object["appName"] = this->name();
      object["time"] = core::TimePoint::clock::to_time_t(core::now());
      m_client.sendCommand(OnlineRoutes::OnlineManager::appStats(), object.dump());
    }

    //-------------------------------------------------------------------------------------------------    

    void Application::enableStats(bool enable) {
      if(initialized()) {
        dqm_error( "Application::enableStats(): Couldn't enable/disable stats, app is already initialized !" );
        throw core::StatusCodeException(core::STATUS_CODE_NOT_ALLOWED);
      }
      m_statsEnabled = enable;
    }
    
    //-------------------------------------------------------------------------------------------------

    bool Application::statsEnabled() const {
      return m_statsEnabled;
    }
    
    //-------------------------------------------------------------------------------------------------

    void Application::init(int argc, char **argv) {
      if(initialized()) {
        dqm_error( "Application::init(): app already initialized !" );
        throw core::StatusCodeException(core::STATUS_CODE_ALREADY_INITIALIZED);        
      }
      try {
        this->parseCmdLine(argc, argv);        
      }
      catch(...) {
        dqm_error( "Application::init(): failed to parse cmd line !" );
        throw core::StatusCodeException(core::STATUS_CODE_FAILURE);
      }
      m_server = std::make_shared<net::Server>(OnlineRoutes::Application::serverName(this->type(), this->name()));
      m_pAppStateService = m_server->createService(OnlineRoutes::Application::state());
      
      if(m_statsEnabled) {
        createInternalStats();
      }
      m_server->onClientExit().connect(this, &Application::sendClientExitEvent);
  
      try {
        this->onInit();        
      }
      catch(...) {
        dqm_error( "Application::init(): failed to initialize the app !" );
        throw core::StatusCodeException(core::STATUS_CODE_FAILURE);
      }
      m_initialized = true;
    }
    
    //-------------------------------------------------------------------------------------------------
    
    int Application::exec() {
      m_running = true;
      m_server->start();
      sleep(1);
      this->setState("Running");
      
      try {
        this->onStart();
      }
      catch(...) {
        dqm_error( "Application::init(): failed to start the app !" );
        throw core::StatusCodeException(core::STATUS_CODE_FAILURE);
      }
      
      int returnCode(0);
      
      try {
        if(m_statsEnabled) {
          // send statistics every 5 seconds
          m_eventLoop.createTimer("AppStats", 5, false, this, &Application::sendAppStats);
        }
        m_eventLoop.connectOnEvent(this, &Application::onEvent);
        returnCode = m_eventLoop.exec();
        this->onStop();
      }
      catch(...) {
        dqm_error( "Application::init(): failed to start the app !" );
        throw core::StatusCodeException(core::STATUS_CODE_FAILURE);
      }      
      std::stringstream state; state << "Exiting (" << returnCode << ")"; 
      this->setState(state.str());
      m_running = false;

      return returnCode;
    }

    //-------------------------------------------------------------------------------------------------

    void Application::exit(int returnCode) {
      m_eventLoop.exit(returnCode);
    }
    
    //-------------------------------------------------------------------------------------------------

    bool Application::initialized() const {
      return m_initialized;
    }

    //-------------------------------------------------------------------------------------------------    

    bool Application::running() const {
      return m_running;
    }
    
    //-------------------------------------------------------------------------------------------------
    
    void Application::queuedSubscribe(const std::string &serviceName, int priority, int maxNEvents) {
      auto findIter = m_serviceHandlerPtrMap.find(serviceName);
      
      if(m_serviceHandlerPtrMap.end() != findIter) {
        dqm_error( "Application::queuedSubscribe(): couldn't subscribe twice to service '{0}'", serviceName );
        throw core::StatusCodeException(core::STATUS_CODE_ALREADY_PRESENT);
      }
      
      auto handler = std::make_shared<NetworkHandler>(m_eventLoop, serviceName, priority, maxNEvents);
      m_serviceHandlerPtrMap.insert(NetworkHandlerPtrMap::value_type(serviceName, handler));
      m_client.subscribe(serviceName, handler.get(), &Application::NetworkHandler::postServiceContent);
    }
    
    //-------------------------------------------------------------------------------------------------
    
    void Application::directSubscribe(const std::string &serviceName) {
      auto findIter = m_serviceHandlerPtrMap.find(serviceName);
      
      if(m_serviceHandlerPtrMap.end() != findIter) {
        dqm_error( "Application::directSubscribe(): couldn't subscribe twice to service '{0}'", serviceName );
        throw core::StatusCodeException(core::STATUS_CODE_ALREADY_PRESENT);
      }
      
      auto handler = std::make_shared<NetworkHandler>(m_eventLoop, serviceName);
      m_serviceHandlerPtrMap.insert(NetworkHandlerPtrMap::value_type(serviceName, handler));
      m_client.subscribe(serviceName, handler.get(), &Application::NetworkHandler::sendServiceContent);
    }
    
    //-------------------------------------------------------------------------------------------------
    
    net::Service *Application::createService(const std::string &name) {
      if(not m_server) {
        dqm_error( "Application::createService(): couldn't create service '{0}', server is not yet allocated", name );
        throw core::StatusCodeException(core::STATUS_CODE_NOT_INITIALIZED);
      }
      return m_server->createService(name);
    }
    
    //-------------------------------------------------------------------------------------------------
    
    void Application::createRequestHandler(const std::string &requestName) {
      if(not m_server) {
        dqm_error( "Application::createRequestHandler(): couldn't create request handler '{0}', server is not yet allocated", requestName );
        throw core::StatusCodeException(core::STATUS_CODE_NOT_INITIALIZED);
      }
      auto findIter = m_requestHandlerPtrMap.find(requestName);
      
      if(m_requestHandlerPtrMap.end() != findIter) {
        dqm_error( "Application::createRequestHandler(): couldn't create twice request handler '{0}'", requestName );
        throw core::StatusCodeException(core::STATUS_CODE_ALREADY_PRESENT);
      }
      
      auto handler = std::make_shared<NetworkHandler>(m_eventLoop, requestName);
      m_serviceHandlerPtrMap.insert(NetworkHandlerPtrMap::value_type(requestName, handler));
      m_server->createRequestHandler(requestName, handler.get(), &Application::NetworkHandler::sendRequestEvent);
    }
    
    //-------------------------------------------------------------------------------------------------
    
    void Application::createQueuedCommand(const std::string &commandName, int priority, int maxNEvents) {
      if(not m_server) {
        dqm_error( "Application::createQueuedCommand(): couldn't create command handler '{0}', server is not yet allocated", commandName );
        throw core::StatusCodeException(core::STATUS_CODE_NOT_INITIALIZED);
      }
      
      auto findIter = m_commandHandlerPtrMap.find(commandName);
      
      if(m_commandHandlerPtrMap.end() != findIter) {
        dqm_error( "Application::createQueuedCommand(): couldn't create command handler '{0}' twice", commandName );
        throw core::StatusCodeException(core::STATUS_CODE_ALREADY_PRESENT);
      }
      
      auto handler = std::make_shared<NetworkHandler>(m_eventLoop, commandName, priority, maxNEvents);
      m_commandHandlerPtrMap.insert(NetworkHandlerPtrMap::value_type(commandName, handler));
      m_server->createCommandHandler(commandName, handler.get(), &Application::NetworkHandler::postCommandEvent);
    }
      
    //-------------------------------------------------------------------------------------------------

    void Application::createDirectCommand(const std::string &commandName) {
      if(not m_server) {
        dqm_error( "Application::createDirectCommand(): couldn't create command handler '{0}', server is not yet allocated", commandName );
        throw core::StatusCodeException(core::STATUS_CODE_NOT_INITIALIZED);
      }
      
      auto findIter = m_commandHandlerPtrMap.find(commandName);
      
      if(m_commandHandlerPtrMap.end() != findIter) {
        dqm_error( "Application::createDirectCommand(): couldn't create command handler '{0}' twice", commandName );
        throw core::StatusCodeException(core::STATUS_CODE_ALREADY_PRESENT);
      }
      
      auto handler = std::make_shared<NetworkHandler>(m_eventLoop, commandName);
      m_commandHandlerPtrMap.insert(NetworkHandlerPtrMap::value_type(commandName, handler));
      m_server->createCommandHandler(commandName, handler.get(), &Application::NetworkHandler::sendCommandEvent);
    }
    
    //-------------------------------------------------------------------------------------------------
    
    int Application::serverClientId() const {
      return m_server->clientId();
    }
    
    //-------------------------------------------------------------------------------------------------
    
    void Application::sendClientExitEvent(int clientId) {
      ClientExitEvent *pEvent = new ClientExitEvent(clientId);
      m_eventLoop.sendEvent(pEvent);
    }
    
    //-------------------------------------------------------------------------------------------------
    
    void Application::removeTimer(const std::string &name) {
      m_eventLoop.removeTimer(name);
    }
    
    //-------------------------------------------------------------------------------------------------
    
    void Application::sendAppStats() {
      core::MemoryStats stats;
      core::memStats(stats);
      
      sendStat("VmProc", stats.vmproc/(1024.));
      sendStat("VmTotal", (stats.vmproc/(stats.vmtot*1.))*(100./1024));
      sendStat("VmInUse", (stats.vmproc/(stats.vmused*1.))*(100./1024));
      sendStat("RSSProc", stats.rssproc/(1024.));
      sendStat("RSSTotal", (stats.rssproc/(stats.rsstot*1.))*(100./1024));
      sendStat("RSSInUse", (stats.rssproc/(stats.rssused*1.))*(100./1024));
    }
    
    //-------------------------------------------------------------------------------------------------
    
    void Application::createInternalStats() {
      // Virtual memory
      createStatsEntry("VmProc", "Mo", "The current virtual memory in use by the application (unit Mo)");
      createStatsEntry("VmTotal", "%", "The current virtual memory percentage in use by the application compare to the total available on the host (unit %)");
      createStatsEntry("VmInUse", "%", "The current virtual memory percentage in use by the application compare to the total used by the running processes (unit %)");
      
      // Resident set size
      createStatsEntry("RSSProc", "Mo", "The current resident set size memory in use by the application (unit Mo)");
      createStatsEntry("RSSTotal", "%", "The current resident set size memory percentage in use by the application compare to the total available on the host (unit %)");
      createStatsEntry("RSSInUse", "%", "The current resident set size memory percentage in use by the application compare to the total used by the running processes (unit %)");
    }
    
    //-------------------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------------------
    
    Application::NetworkHandler::NetworkHandler(AppEventLoop &eventLoop, const std::string &name, int priority, int maxNEvents) :
      m_eventLoop(eventLoop),
      m_name(name),
      m_priority(priority),
      m_maxNEvents(maxNEvents)
    {
    }
    
    //-------------------------------------------------------------------------------------------------
    
    void Application::NetworkHandler::postServiceContent(const net::Buffer &buffer) {
      int count = m_eventLoop.count([this](std::shared_ptr<AppEvent> ptr){
        ServiceUpdateEvent *evt = dynamic_cast<ServiceUpdateEvent*>(ptr.get());
        return (evt && evt->serviceName() == this->m_name);
      });
      
      if(count >= m_maxNEvents) {
        dqm_debug( "NetworkHandler::postServiceContent(): maximum of posted service updates reached ({0}) !", m_maxNEvents );
        return;
      }
      
      // copy buffer content
      std::string content;
      content.assign(buffer.begin(), buffer.size());
      
      // create new buffer model
      auto bufferModel = buffer.createModel<std::string>();
      bufferModel->move(std::move(content));
      
      // create the event to post, pass the copied buffer in ctor
      ServiceUpdateEvent *pEvent = new ServiceUpdateEvent(m_name, bufferModel);
      pEvent->setPriority(m_priority);
      
      // post the event !
      m_eventLoop.postEvent(pEvent);
    }
    
    //-------------------------------------------------------------------------------------------------
    
    void Application::NetworkHandler::sendServiceContent(const net::Buffer &buffer) {
      ServiceUpdateEvent *pEvent = new ServiceUpdateEvent(m_name, buffer.model());
      m_eventLoop.sendEvent(pEvent);
    }
    
    //-------------------------------------------------------------------------------------------------
    
    void Application::NetworkHandler::sendRequestEvent(const net::Buffer &request, net::Buffer &response) {
      RequestEvent *pEvent = new RequestEvent(m_name, request.model(), response);
      m_eventLoop.sendEvent(pEvent);
    }
    
    //-------------------------------------------------------------------------------------------------
    
    void Application::NetworkHandler::postCommandEvent(const net::Buffer &buffer) {
      int count = m_eventLoop.count([this](std::shared_ptr<AppEvent> ptr){
        CommandEvent *evt = dynamic_cast<CommandEvent*>(ptr.get());
        return (evt && evt->commandName() == this->m_name);
      });
      
      if(count >= m_maxNEvents) {
        dqm_debug( "NetworkHandler::postCommandEvent(): maximum of posted command handling reached ({0}) !", m_maxNEvents );
        return;
      }
      
      // copy buffer content
      std::string content;
      content.assign(buffer.begin(), buffer.size());
      
      // create new buffer model
      auto bufferModel = buffer.createModel<std::string>();
      bufferModel->move(std::move(content));
      
      // create the event to post, pass the copied buffer in ctor
      CommandEvent *pEvent = new CommandEvent(m_name, bufferModel);
      pEvent->setPriority(m_priority);
      
      // post the event !
      m_eventLoop.postEvent(pEvent);
    }
    
    //-------------------------------------------------------------------------------------------------

    void Application::NetworkHandler::sendCommandEvent(const net::Buffer &buffer) {
      CommandEvent *pEvent = new CommandEvent(m_name, buffer.model());
      m_eventLoop.sendEvent(pEvent);
    }
    
  }

}

