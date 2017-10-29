/// \file RunControl.h
/*
 *
 * DQMRunControl.h header template automatically generated by a class generator
 * Creation date : mar. oct. 7 2014
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


#ifndef DQM4HEP_RUNCONTROL_H
#define DQM4HEP_RUNCONTROL_H

// -- dqm4hep headers
#include <dqm4hep/Internal.h>
#include <dqm4hep/StatusCodes.h>
#include <dqm4hep/Signal.h>
#include <dqm4hep/Run.h>

namespace dqm4hep {

  namespace online {

    using StartOfRunSignal = dqm4hep::core::Signal<dqm4hep::core::Run &>;
    using EndOfRunSignal = dqm4hep::core::Signal<const dqm4hep::core::Run &>;

    /** RunControl class
     */
    class RunControl
    {
    public:
      /** Constructor
       */
      RunControl();

      /** Constructor with name
       */
      RunControl(const std::string &name);

      /** Destructor
       */
      ~RunControl();

      /** Set the run control name
       */
      dqm4hep::core::StatusCode setName(const std::string &name);

      /** Get the run control name
       */
      const std::string &name() const;

      /** Create a new run from a Run
       *  The run control is the owner the run
       */
      dqm4hep::core::StatusCode startNewRun(const dqm4hep::core::Run &run, const std::string &password = "");

      /** Create a new run.
       *  End the current run if running
       */
      dqm4hep::core::StatusCode startNewRun(int runNumber, const std::string &description = "", const std::string &detectorName = "", const std::string &password = "");

      /** End the current run
       */
      dqm4hep::core::StatusCode endCurrentRun(const std::string &password = "");

      /** Get the current run
       */
      const dqm4hep::core::Run &currentRun() const;

      /** Get the current run
       */
      dqm4hep::core::Run &currentRun();

      /** Whether a run has been started.
       */
      bool isRunning() const;

      /** Set the password needed to start/stop runs
       */
      void setPassword( const std::string &password );

      /** Check run control password
       */
      bool checkPassword(const std::string &password);

      /**
       */
      StartOfRunSignal &onStartOfRun();

      /**
       */
      EndOfRunSignal &onEndOfRun();

    protected:
      bool                             m_running;
      dqm4hep::core::Run               m_run;
      std::string                      m_name;
      std::string                      m_password;
      StartOfRunSignal                 m_sorSignal;
      EndOfRunSignal                   m_eorSignal;
    };

  }

}

#endif  //  DQM4HEP_RUNCONTROL_H
