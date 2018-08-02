/*

Simple VOIP Wrap.

Copyright (C) 2018 Sergey Kolevatov

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

*/


// $Revision: 9580 $ $Date:: 2018-08-02 #$ $Author: serge $

#ifndef SIMPLE_VOIP_WRAP__WRAP_H
#define SIMPLE_VOIP_WRAP__WRAP_H

#include <list>                             // std::list
#include <mutex>                            // std::mutex
#include <map>                              // std::map
#include <set>                              // std::set

#include "simple_voip/objects.h"            // simple_voip::InitiateCallRequest
#include "simple_voip/i_simple_voip.h"      // simple_voip::ISimpleVoip
#include "simple_voip/i_simple_voip_callback.h" // ISimpleVoipCallback

#include "namespace_lib.h"              // namespace simple_voip_wrap {

namespace simple_voip_wrap {

class Wrap:
    virtual public simple_voip::ISimpleVoip,
    virtual public simple_voip::ISimpleVoipCallback
{
public:
    Wrap();
    ~Wrap();

    bool init(
            unsigned int                        log_id,
            simple_voip::ISimpleVoip            * voips,
            simple_voip::ISimpleVoipCallback    * callback,
            std::string                         * error_msg );

    // interface ISimpleVoip
    void consume( const simple_voip::ForwardObject* obj );

    // interface ISimpleVoipCallback
    void consume( const simple_voip::CallbackObject * obj );

    // interface threcon::IControllable
    bool shutdown();

private:

    typedef std::set<uint32_t>              SetReqIds;

private:

    // simple_voip::ISimpleVoip interface
    void handle_InitiateCallRequest( const simple_voip::ForwardObject * req );
    void handle_DropRequest( const simple_voip::ForwardObject * req );

    // interface ISimpleVoipCallback
    void handle_InitiateCallResponse( const simple_voip::CallbackObject * obj );
    void handle_RejectResponse( const simple_voip::CallbackObject * obj );
    void handle_ErrorResponse( const simple_voip::CallbackObject * obj );
    void handle_DropResponse( const simple_voip::CallbackObject * obj );
    void handle_ConnectionLost( const simple_voip::CallbackObject * obj );
    void handle_Failed( const simple_voip::CallbackObject * obj );

private:
    mutable std::mutex          mutex_;

    unsigned int                log_id_;

    simple_voip::ISimpleVoip            * voips_;
    simple_voip::ISimpleVoipCallback    * callback_;

    SetReqIds                   play_file_req_ids_;
    SetReqIds                   play_file_stop_req_ids_;
    SetReqIds                   record_file_req_ids_;
    SetReqIds                   record_file_stop_req_ids_;
};

} // namespace simple_voip_wrap

#endif  // SIMPLE_VOIP_WRAP__WRAP_H
