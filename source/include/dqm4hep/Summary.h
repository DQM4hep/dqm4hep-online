/// \file Summary.h
/*
*
* Summary.h header template automatically generated by a class generator
* Creation date : jeu. f�vr. 2 2017
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


#ifndef SUMMARY_H
#define SUMMARY_H

#include "dqm4hep/DQM4HEP.h"
#include "dqm4hep/MonitorObject.h"
#include "dqm4hep/DrawAttributes.h"

namespace dqm4hep {

  namespace core {

    /**
    * @brief Summary class
    */
    class Summary final : public MonitorObject
    {
    public:
      /**
      * Property enum
      */
      enum Property
      {
        HEADER = 0,
        ENTRIES,
        N_PROPERTIES
      };

      /**
      * Constructor
      */
      Summary();

      /**
      * Constructor
      */
      Summary(const std::string &header);
      
      /**
      * Destructor
      */
      ~Summary();

      /**
      * [create description]
      * @param  value [description]
      * @return       [description]
      */
      static Summary *create(const Json::Value &value);

      /**
      * [setHeader description]
      * @param header [description]
      */
      void setHeader(const std::string &header);

      /**
      * [getHeader description]
      * @return [description]
      */
      const std::string &getHeader() const;

      /**
      * [setEntry description]
      * @param entry [description]
      * @param text  [description]
      */
      void setEntry(const std::string &entry, const std::string &text);

      /**
      * [removeEntry description]
      * @param entry [description]
      */
      void removeEntry(const std::string &entry);

      /**
      * [clear description]
      */
      void clear();

      // from monitor object
      bool isUpToDate() const;
      void fromJson(const Json::Value &value);
      void toJson(Json::Value &value, bool full = true, bool resetCache = true);
      MonitorObjectType getType() const;

    private:
      void resetCache();

      std::bitset<N_PROPERTIES>               m_updateCache;
      std::string                             m_header;
      std::map<std::string, std::string>      m_entries;
    };

  }

}

#endif  //  SUMMARY_H
