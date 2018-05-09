/// \file test-online-element.cc
/*
 *
 * test-online-element.cc main source file template automatically generated
 * Creation date : mer. nov. 5 2014
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
#include <dqm4hep/Internal.h>
#include <dqm4hep/Logging.h>
#include <dqm4hep/MonitorElementManager.h>
#include <dqm4hep/OnlineElement.h>
#include <dqm4hep/StatusCodes.h>

// -- std headers
#include <iostream>
#include <signal.h>

using namespace std;
using namespace dqm4hep::core;
using namespace dqm4hep::online;

#define assert_test(Command)                                                                                           \
  if (!(Command)) {                                                                                                    \
    dqm_error("Assertion failed : {0}, line {1}", #Command, __LINE__);                                                 \
    exit(1);                                                                                                           \
  }

int main(int /*argc*/, char ** /*argv*/) {
  Logger::createLogger("test-online-element", {Logger::coloredConsole()});
  Logger::setMainLogger("test-online-element");
  Logger::setLogLevel(spdlog::level::debug);

  std::unique_ptr<MonitorElementManager> meMgr = std::unique_ptr<MonitorElementManager>(new MonitorElementManager());

  OnlineElementPtr monitorElement;
  OnlineElementPtrList monitorElements;
  assert_test(STATUS_CODE_SUCCESS == meMgr->bookMonitorElement("TGraph", "/", "TestGraph", monitorElement));

  meMgr->getMonitorElements(monitorElements);
  assert_test(monitorElements.size() == 1);
  monitorElements.clear();

  assert_test(STATUS_CODE_ALREADY_PRESENT == meMgr->bookMonitorElement("TGraph", "/", "TestGraph", monitorElement));
  assert_test(STATUS_CODE_SUCCESS == meMgr->getMonitorElement("/", "TestGraph", monitorElement));
  assert_test(STATUS_CODE_NOT_FOUND == meMgr->getMonitorElement("/", "TestGraph2", monitorElement));

  assert_test(STATUS_CODE_SUCCESS == meMgr->removeMonitorElement("/", "TestGraph"));
  assert_test(STATUS_CODE_NOT_FOUND == meMgr->getMonitorElement("/", "TestGraph", monitorElement));

  return 0;
}
