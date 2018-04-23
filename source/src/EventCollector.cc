/*
 *
 * DQMEventCollector.cc source template automatically generated by a class generator
 * Creation date : mer. sept. 9 2015
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
#include "dqm4hep/EventCollector.h"
#include "DQMOnlineConfig.h"
#include "dqm4hep/Logging.h"
#include "dqm4hep/OnlineRoutes.h"

namespace dqm4hep {

  namespace online {
    
    EventCollector::EventCollector() : 
      Application() {
    }

    //-------------------------------------------------------------------------------------------------

    void EventCollector::parseCmdLine(int argc, char **argv) {
      std::string cmdLineFooter = "Please report bug to <dqm4hep@gmail.com>";
      m_cmdLine = std::make_shared<TCLAP::CmdLine>(cmdLineFooter, ' ', DQMOnline_VERSION_STR);
      
      TCLAP::ValueArg<std::string> collectorNameArg(
          "c"
          , "collector-name"
          , "The event collector name"
          , true
          , ""
          , "string");
      m_cmdLine->add(collectorNameArg);
      
      core::StringVector verbosities(core::Logger::logLevels());
      TCLAP::ValuesConstraint<std::string> verbosityConstraint(verbosities);
      TCLAP::ValueArg<std::string> verbosityArg(
          "v"
          , "verbosity"
          , "The logging verbosity"
          , false
          , "info"
          , &verbosityConstraint);
      m_cmdLine->add(verbosityArg);
      
      // parse command line
      m_cmdLine->parse(argc, argv);

      std::string verbosity(verbosityArg.getValue());
      std::string collectorName(collectorNameArg.getValue());
      setType(OnlineRoutes::EventCollector::applicationType());
      setName(collectorName);
      setLogLevel(core::Logger::logLevelFromString(verbosity));
    }
    
    //-------------------------------------------------------------------------------------------------
    
    void EventCollector::onInit() {
      // create network services
      createRequestHandler(
        OnlineRoutes::EventCollector::registerSource(name()), 
        this, 
        &EventCollector::handleRegistration
      );
      createDirectCommand(
        OnlineRoutes::EventCollector::unregisterSource(name()), 
        this, 
        &EventCollector::handleClientUnregistration
      );
      createDirectCommand(
        OnlineRoutes::EventCollector::collectEvent(name()), 
        this, 
        &EventCollector::handleCollectEvent
      );
      
      // create statistics entries
      createStatsEntry("NSources", "", "The current number of registered sources");
      createStatsEntry("NEvents_60sec", "1/min", "The number of collected events within the last minute");
      createStatsEntry("NEvents_10sec", "1/10 sec", "The number of collected events within the last 10 secondes");
      createStatsEntry("NBytes_60sec", "bytes", "The total number of collected bytes within the last minute");
      createStatsEntry("NBytes_10sec", "bytes", "The total number of collected bytes within the last 10 secondes");
      createStatsEntry("NMeanBytes_60sec", "bytes/min", "The mean number of collected bytes within the last minute");
      createStatsEntry("NMeanBytes_10sec", "bytes/10 sec", "The mean number of collected bytes within the last 10 secondes");
      
      // app stats timers
      createTimer("CollectorStats10Secs", 10, false, this, &EventCollector::sendStatsTimer10);
      createTimer("CollectorStats60Secs", 60, false, this, &EventCollector::sendStatsTimer60);
      m_lastStatCall10 = core::now();
      m_lastStatCall60 = core::now();
    }
    
    //-------------------------------------------------------------------------------------------------
    
    void EventCollector::onEvent(AppEvent *pAppEvent) {
      if(pAppEvent->type() == AppEvent::CLIENT_EXIT) {
        ClientExitEvent *pClientExitEvent = (ClientExitEvent *) pAppEvent;
        this->handleClientExit(pClientExitEvent);
      }
    }
    
    //-------------------------------------------------------------------------------------------------
    
    void EventCollector::onStart() {
    }
    
    //-------------------------------------------------------------------------------------------------
    
    void EventCollector::onStop() {
    }
    
    //-------------------------------------------------------------------------------------------------
    
    void EventCollector::handleRegistration(const net::Buffer &request, net::Buffer &response) {
      core::json registrationDetails({});
      if(0 != request.size()) {
        registrationDetails = core::json::parse(request.begin(), request.end());   
      }
      auto clientSourceName = registrationDetails.value<std::string>("source", "");
      auto clientId = this->serverClientId();  
      auto findIter = m_sourceInfoMap.find(clientSourceName);
      core::json clientResponseValue({});
      
      // source already registered
      if(m_sourceInfoMap.end() != findIter) {
        if(clientId == findIter->second.m_clientId) {
          clientResponseValue["message"] = "Event source already registered !";
          clientResponseValue["registered"] = true;
        }
        else {
          std::stringstream ss; ss << "Event source already registered with a different client ID (" << findIter->second.m_clientId << ") !";
          clientResponseValue["message"] = ss.str();
          clientResponseValue["registered"] = false;
        }
      }
      // source not registered yet
      else {
        std::string sourceName = registrationDetails.value<std::string>("source", "");
        SourceInfo sourceInfo;
        findIter = m_sourceInfoMap.insert(SourceInfoMap::value_type(sourceName, std::move(sourceInfo))).first;
        
        findIter->second.m_clientId = clientId;
        findIter->second.m_name = registrationDetails.value<std::string>("source", "");
        findIter->second.m_streamerName = registrationDetails.value<std::string>("streamer", "");
        
        auto collectors = registrationDetails["collectors"];
        auto hostInfo = registrationDetails["host"];
        
        for(auto collector : collectors) {
          findIter->second.m_collectors.push_back(collector.get<std::string>());
        }
          
        for(auto hostInfoIter : hostInfo.items()) {
          findIter->second.m_hostInfo[hostInfoIter.key()] = hostInfo[hostInfoIter.key()].get<std::string>();
        }
        
        auto model = findIter->second.m_buffer.createModel<std::string>();
        findIter->second.m_buffer.setModel(model);
        
        dqm_info( "New event source '{0}' registered with client id {1}", findIter->second.m_name, findIter->second.m_clientId );
        
        clientResponseValue["registered"] = true;
        sendStat("NSources", m_sourceInfoMap.size());
      }
      
      auto model = response.createModel<std::string>();
      model->copy(clientResponseValue.dump());
      response.setModel(model);
    }
    
    //-------------------------------------------------------------------------------------------------
    
    void EventCollector::handleClientExit(ClientExitEvent *event) {
      const int clientId(event->clientId());
      auto findIter = std::find_if(m_sourceInfoMap.begin(), m_sourceInfoMap.end(), [&clientId](const SourceInfoMap::value_type &iter){
        return (iter.second.m_clientId == clientId);
      });
      
      if(findIter != m_sourceInfoMap.end()) {
        dqm_info( "Removing event source '{0}' from source list !", findIter->second.m_name );
        m_sourceInfoMap.erase(findIter);
        sendStat("NSources", m_sourceInfoMap.size());
      }
    }
    
    //-------------------------------------------------------------------------------------------------
    
    void EventCollector::handleCollectEvent(const net::Buffer &buffer) {
      const int clientId(serverClientId());
      
      auto findIter = std::find_if(m_sourceInfoMap.begin(), m_sourceInfoMap.end(), [&clientId](const SourceInfoMap::value_type &iter){
        return (iter.second.m_clientId == clientId);
      });
      
      if(findIter != m_sourceInfoMap.end()) {          
        std::string copiedBuffer(buffer.begin(), buffer.size());
        auto newModel = findIter->second.m_buffer.createModel<std::string>();
        findIter->second.m_buffer.setModel(newModel);
        newModel->move(std::move(copiedBuffer));
        
        m_nCollectedEvents10++;
        m_nCollectedEvents60++;
        m_nCollectedBytes10 += buffer.size();
        m_nCollectedBytes60 += buffer.size();
      }
    }
    
    //-------------------------------------------------------------------------------------------------
    
    void EventCollector::handleClientUnregistration(const net::Buffer &/*buffer*/) {
      const int clientId(serverClientId());
      auto findIter = std::find_if(m_sourceInfoMap.begin(), m_sourceInfoMap.end(), [&clientId](const SourceInfoMap::value_type &iter){
        return (iter.second.m_clientId == clientId);
      });
      
      if(findIter != m_sourceInfoMap.end()) {
        dqm_info( "Removing event source '{0}' from source list !", findIter->second.m_name );
        m_sourceInfoMap.erase(findIter);
        sendStat("NSources", m_sourceInfoMap.size());
      }
    }
    
    //-------------------------------------------------------------------------------------------------
    
    void EventCollector::sendStatsTimer10() {
      auto timeDifference = std::chrono::duration_cast<std::chrono::milliseconds>(core::now()-m_lastStatCall10).count();
      // send stats
      sendStat("NEvents_10sec", m_nCollectedEvents10);
      sendStat("NBytes_10sec", m_nCollectedBytes10);
      sendStat("NMeanBytes_10sec", m_nCollectedBytes10 / (timeDifference/1000.));
      // reset counters
      m_nCollectedEvents10 = 0;
      m_nCollectedBytes10 = 0;
      m_lastStatCall10 = core::now();
    }
    
    //-------------------------------------------------------------------------------------------------
    
    void EventCollector::sendStatsTimer60() {
      auto timeDifference = std::chrono::duration_cast<std::chrono::milliseconds>(core::now()-m_lastStatCall60).count();
      // send stats
      sendStat("NEvents_60sec", m_nCollectedEvents60);
      sendStat("NBytes_60sec", m_nCollectedBytes60);
      sendStat("NMeanBytes_60sec", m_nCollectedBytes60 / (timeDifference/1000.));
      // reset counters
      m_nCollectedEvents60 = 0;
      m_nCollectedBytes60 = 0;
      m_lastStatCall60 = core::now();
    }
    
    //-------------------------------------------------------------------------------------------------
    
    void EventCollector::printSourceMap() {
      for(auto &source : m_sourceInfoMap) {
        dqm_debug( "== Source '{0}' ==", source.first );
        dqm_debug( "     Client id: '{0}' ==", source.second.m_clientId );
        dqm_debug( "     Streamer:  '{0}' ==", source.second.m_streamerName );
      }
    }
    
    //-------------------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------------------
    
    EventCollector::SourceInfo::SourceInfo(EventCollector::SourceInfo&& info) :
      m_clientId(std::move(info.m_clientId)),
      m_name(std::move(info.m_name)),
      m_streamerName(std::move(info.m_streamerName)),
      m_collectors(std::move(info.m_collectors)),
      m_hostInfo(std::move(info.m_hostInfo)),
      m_buffer(std::move(info.m_buffer)) {
        /* nop */
    }

  }

}
