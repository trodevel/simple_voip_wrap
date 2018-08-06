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


// $Revision: 9603 $ $Date:: 2018-08-06 #$ $Author: serge $

#ifndef SIMPLE_VOIP_WRAP__WRAP_H
#define SIMPLE_VOIP_WRAP__WRAP_H

#include <list>                             // std::list
#include <mutex>                            // std::mutex
#include <map>                              // std::map

#include "scheduler/i_scheduler.h"          // IScheduler
#include "simple_voip/objects.h"            // simple_voip::InitiateCallRequest
#include "simple_voip/i_simple_voip.h"      // simple_voip::ISimpleVoip
#include "simple_voip/i_simple_voip_callback.h" // ISimpleVoipCallback
#include "utils/i_request_id_gen.h"         // utils::IRequestIdGen

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
            scheduler::IScheduler               * scheduler,
            utils::IRequestIdGen                * req_id_gen,
            std::string                         * error_msg );

    // interface ISimpleVoip
    void consume( const simple_voip::ForwardObject* obj );

    // interface ISimpleVoipCallback
    void consume( const simple_voip::CallbackObject * obj );

    // interface threcon::IControllable
    bool shutdown();

    void release_message( const simple_voip::ForwardObject* obj );
    void release_message( const simple_voip::CallbackObject* obj );

private:

    enum class type_e
    {
        PlayFileRequest,
        PlayFileStopRequest,
        RecordFileRequest,
        RecordFileStopRequest
    };

    typedef struct Param
    {
        type_e      type;
        uint32_t    req_id;
        uint32_t    call_id;
        double      duration;
        std::string filename;

        void init(
            type_e              type,
            uint32_t            req_id,
            uint32_t            call_id,
            double              duration,
            const std::string   & filename )
        {
            this->type      = type;
            this->req_id    = req_id;
            this->call_id   = call_id;
            this->duration  = duration;
            this->filename  = filename;
        }
    };

    typedef std::map<uint32_t, Param>      MapReqIdToParam;

private:

    // simple_voip::ISimpleVoip interface
    void handle_PlayFileRequest( const simple_voip::ForwardObject * req );
    void handle_RecordFileRequest( const simple_voip::ForwardObject * req );

    // interface ISimpleVoipCallback
    void handle_RejectResponse( const simple_voip::CallbackObject * obj );
    void handle_ErrorResponse( const simple_voip::CallbackObject * obj );
    void handle_PlayFileResponse( const simple_voip::CallbackObject * obj );
    void handle_PlayFileStopResponse( const simple_voip::CallbackObject * obj );
    void handle_RecordFileResponse( const simple_voip::CallbackObject * obj );
    void handle_RecordFileStopResponse( const simple_voip::CallbackObject * obj );

    void handle_error( type_e type, uint32_t req_id, uint32_t orig_req_id, uint32_t errorcode, const std::string & error_msg );

    void schedule_stop_event( uint32_t req_id, uint32_t call_id, double duration, bool is_record );

    void handle_generate_play_stop( uint32_t start_req_id, uint32_t call_id );
    void handle_generate_record_stop( uint32_t start_req_id, uint32_t call_id );

    double get_duration( const std::string & filename );

private:
    mutable std::mutex          mutex_;

    unsigned int                log_id_;

    simple_voip::ISimpleVoip            * voips_;
    simple_voip::ISimpleVoipCallback    * callback_;
    scheduler::IScheduler               * scheduler_;
    utils::IRequestIdGen                * req_id_gen_;

    MapReqIdToParam             map_req_to_param_;
};

} // namespace simple_voip_wrap

#endif  // SIMPLE_VOIP_WRAP__WRAP_H
