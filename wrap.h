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

// $Revision: 9710 $ $Date:: 2018-09-06 #$ $Author: serge $

#ifndef SIMPLE_VOIP_WRAP__WRAP_H
#define SIMPLE_VOIP_WRAP__WRAP_H

#include <mutex>                            // std::mutex
#include <map>                              // std::map

#include "scheduler/i_scheduler.h"          // IScheduler
#include "objects.h"                        // simple_voip::InitiateCallRequest
#include "simple_voip/i_simple_voip.h"      // simple_voip::ISimpleVoip
#include "simple_voip/i_simple_voip_callback.h" // ISimpleVoipCallback
#include "utils/i_request_id_gen.h"         // utils::IRequestIdGen

#include "i_get_duration.h"                 // IGetDuration

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
            IGetDuration                        * gd,
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

    struct Param
    {
        type_e      type;
        uint32_t    start_req_id;
        uint32_t    call_id;
        double      duration;
        std::string filename;

        void init(
            type_e              type,
            uint32_t            start_req_id,
            uint32_t            call_id,
            double              duration,
            const std::string   & filename )
        {
            this->type      = type;
            this->start_req_id  = start_req_id;
            this->call_id   = call_id;
            this->duration  = duration;
            this->filename  = filename;
        }
    };

    typedef std::map<uint32_t, Param>      MapReqIdToParam;

private:

    uint32_t get_resp_id( const simple_voip::CallbackObject * obj );

    // simple_voip::ISimpleVoip interface
    void handle_PlayFileRequest( const simple_voip::ForwardObject * req );
    void handle_RecordFileRequest( const simple_voip::ForwardObject * req );

    void handle( const simple_voip::CallbackObject * obj, const Param & p );

    // interface ISimpleVoipCallback
    void handle_RejectResponse( const simple_voip::CallbackObject * obj, const Param & p );
    void handle_ErrorResponse( const simple_voip::CallbackObject * obj, const Param & p );
    void handle_PlayFileResponse( const simple_voip::CallbackObject * obj, const Param & p );
    void handle_PlayFileStopResponse( const simple_voip::CallbackObject * obj, const Param & p );
    void handle_RecordFileResponse( const simple_voip::CallbackObject * obj, const Param & p );
    void handle_RecordFileStopResponse( const simple_voip::CallbackObject * obj, const Param & p );

    void handle_error( type_e type, uint32_t req_id, uint32_t orig_req_id, uint32_t call_id, uint32_t errorcode, const std::string & error_msg );

    bool schedule_stop_event( uint32_t req_id, uint32_t call_id, double duration, bool is_record );

    void handle_generate_play_stop( uint32_t start_req_id, uint32_t call_id );
    void handle_generate_record_stop( uint32_t start_req_id, uint32_t call_id );

private:
    mutable std::mutex          mutex_;

    unsigned int                log_id_;

    simple_voip::ISimpleVoip            * voips_;
    simple_voip::ISimpleVoipCallback    * callback_;
    scheduler::IScheduler               * scheduler_;
    utils::IRequestIdGen                * req_id_gen_;
    IGetDuration                        * gd_;

    MapReqIdToParam             map_req_to_param_;
};

} // namespace simple_voip_wrap

#endif  // SIMPLE_VOIP_WRAP__WRAP_H
