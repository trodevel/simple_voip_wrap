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

// $Revision: 9641 $ $Date:: 2018-08-08 #$ $Author: serge $

#include "wrap.h"                       // self

#include <typeindex>                    // std::type_index
#include <typeinfo>
#include <unordered_map>

#include "utils/mutex_helper.h"         // MUTEX_SCOPE_LOCK
#include "utils/dummy_logger.h"         // dummy_log
#include "utils/assert.h"               // ASSERT
#include "scheduler/timeout_job_aux.h"  // create_and_insert_timeout_job

#include "simple_voip/object_factory.h" // create_play_file_request

#include "object_factory.h"             // create_PlayFileErrorResponse
#include "objects.h"                    // PlayFileRequest
#include "str_helper.h"                 // StrHelper

#define MODULENAME      "Wrap"

namespace simple_voip_wrap {

Wrap::Wrap():
    log_id_( 0 ),
    voips_( nullptr ), callback_( nullptr ),
    scheduler_( nullptr ),
    req_id_gen_( nullptr ),
    gd_( nullptr )
{
}

Wrap::~Wrap()
{
}

bool Wrap::init(
        unsigned int                        log_id,
        simple_voip::ISimpleVoip            * voips,
        simple_voip::ISimpleVoipCallback    * callback,
        scheduler::IScheduler               * scheduler,
        utils::IRequestIdGen                * req_id_gen,
        IGetDuration                        * gd,
        std::string                         * error_msg )
{
    if( voips == nullptr )
        return false;

    if( callback == nullptr )
        return false;

    MUTEX_SCOPE_LOCK( mutex_ );

    if( voips_ != nullptr )
        return false;

    if( callback_ != nullptr )
        return false;

    log_id_     = log_id;
    voips_      = voips;
    callback_   = callback;
    scheduler_  = scheduler;
    req_id_gen_ = req_id_gen;
    gd_         = gd;

    return true;
}

void Wrap::consume( const simple_voip::ForwardObject* obj )
{
    MUTEX_SCOPE_LOCK( mutex_ );

    typedef Wrap Type;

    typedef void (Type::*PPMF)( const simple_voip::ForwardObject * r );

#define HANDLER_MAP_ENTRY(_v)       { typeid( simple_voip::wrap::_v ),            & Type::handle_##_v }

    static const std::unordered_map<std::type_index, PPMF> funcs =
    {
        HANDLER_MAP_ENTRY( PlayFileRequest ),
        HANDLER_MAP_ENTRY( RecordFileRequest ),
    };

#undef HANDLER_MAP_ENTRY

    auto it = funcs.find( typeid( * obj ) );

    if( it != funcs.end() )
    {
        (this->*it->second)( obj );

        release_message( obj );
    }
    else
    {
        voips_->consume( obj );
    }
}

void Wrap::consume( const simple_voip::CallbackObject* obj )
{
    MUTEX_SCOPE_LOCK( mutex_ );

    auto req_id = get_resp_id( obj );

    auto it = map_req_to_param_.find( req_id );

    if( it == map_req_to_param_.end() )
    {
        callback_->consume( obj );
        return;
    }

    auto & p = it->second;

    handle( obj, p );

    map_req_to_param_.erase( it );
    release_message( obj );
}

void Wrap::handle( const simple_voip::CallbackObject* obj, const Param & p )
{
    dummy_log_trace( log_id_, "handle(): %s", StrHelper::to_string( *obj ).c_str() );

    typedef Wrap Type;

    typedef void (Type::*PPMF)( const simple_voip::CallbackObject * r, const Param & p );

#define HANDLER_MAP_ENTRY(_v)       { typeid( simple_voip::_v ),            & Type::handle_##_v }

    static const std::unordered_map<std::type_index, PPMF> funcs =
    {
        HANDLER_MAP_ENTRY( ErrorResponse ),
        HANDLER_MAP_ENTRY( RejectResponse ),
        HANDLER_MAP_ENTRY( PlayFileResponse ),
        HANDLER_MAP_ENTRY( PlayFileStopResponse ),
        HANDLER_MAP_ENTRY( RecordFileResponse ),
        HANDLER_MAP_ENTRY( RecordFileStopResponse ),
    };

#undef HANDLER_MAP_ENTRY

    auto it = funcs.find( typeid( * obj ) );

    if( it != funcs.end() )
    {
        (this->*it->second)( obj, p );
    }
    else
    {
        dummy_log_error( log_id_, "handle(): unknown type %s", typeid( *obj ).name() );
        ASSERT( false );
    }
}

bool Wrap::shutdown()
{
    dummy_log_debug( log_id_, "shutdown()" );

    MUTEX_SCOPE_LOCK( mutex_ );

    return true;
}

void Wrap::release_message( const simple_voip::ForwardObject* obj )
{
    delete obj;
}

void Wrap::release_message( const simple_voip::CallbackObject* obj )
{
    delete obj;
}

void Wrap::handle_PlayFileRequest( const simple_voip::ForwardObject * rreq )
{
    auto * req = dynamic_cast< const simple_voip::wrap::PlayFileRequest *>( rreq );

    Param p;

    p.init( type_e::PlayFileRequest, req->req_id, req->call_id, 0, req->filename );

    auto b = map_req_to_param_.insert( std::make_pair( req->req_id, p ) ).second;

    ASSERT( b );

    auto req2 = simple_voip::create_play_file_request( req->req_id, req->call_id, req->filename );

    voips_->consume( req2 );
}

void Wrap::handle_RecordFileRequest( const simple_voip::ForwardObject * rreq )
{
    auto * req = dynamic_cast< const simple_voip::wrap::RecordFileRequest *>( rreq );

    // private: no mutex lock

    Param p;

    p.init( type_e::RecordFileRequest, req->req_id, req->call_id, req->duration, req->filename );

    auto b = map_req_to_param_.insert( std::make_pair( req->req_id, p ) ).second;

    ASSERT( b );

    auto req2 = simple_voip::create_record_file_request( req->req_id, req->call_id, req->filename );

    voips_->consume( req2 );
}

uint32_t Wrap::get_resp_id( const simple_voip::CallbackObject * obj )
{
    auto & type = typeid( *obj );

#define GET_RESP_ID_IF_TYPE(_v) if( type == typeid( simple_voip::_v ) ) { return dynamic_cast< const simple_voip::_v *>( obj )->req_id; }

    GET_RESP_ID_IF_TYPE( ErrorResponse ) else
    GET_RESP_ID_IF_TYPE( RejectResponse ) else
    GET_RESP_ID_IF_TYPE( PlayFileResponse ) else
    GET_RESP_ID_IF_TYPE( PlayFileStopResponse ) else
    GET_RESP_ID_IF_TYPE( RecordFileResponse ) else
    GET_RESP_ID_IF_TYPE( RecordFileStopResponse );

    return 0;
}

void Wrap::handle_error( type_e type, uint32_t req_id, uint32_t orig_req_id, uint32_t errorcode, const std::string & error_msg )
{
    switch( type )
    {
    case type_e::PlayFileRequest:
    {
        auto * resp = simple_voip::wrap::create_PlayFileErrorResponse( req_id, errorcode, error_msg );

        callback_->consume( resp );
    }
        break;

    case type_e::PlayFileStopRequest:   // despite the error return a correct response
    {
        auto * resp = simple_voip::wrap::create_PlayFileStopped( orig_req_id );

        callback_->consume( resp );
    }
        break;

    case type_e::RecordFileRequest:
    {
        auto * resp = simple_voip::wrap::create_RecordFileErrorResponse( req_id, errorcode, error_msg );

        callback_->consume( resp );
    }
        break;

    case type_e::RecordFileStopRequest: // despite the error return a correct response
    {
        auto * resp = simple_voip::wrap::create_RecordFileStopped( orig_req_id );

        callback_->consume( resp );
    }
        break;

    default:
        break;
    }
}

// ISimpleVoipCallback interface
void Wrap::handle_RejectResponse( const simple_voip::CallbackObject * oobj, const Param & p )
{
    auto * obj = dynamic_cast< const simple_voip::RejectResponse *>( oobj );

    handle_error( p.type, obj->req_id, p.start_req_id, 0, "rejected" );
}

void Wrap::handle_ErrorResponse( const simple_voip::CallbackObject * oobj, const Param & p )
{
    auto * obj = dynamic_cast< const simple_voip::ErrorResponse *>( oobj );

    handle_error( p.type, obj->req_id, p.start_req_id, obj->errorcode, obj->descr );
}

void Wrap::handle_PlayFileResponse( const simple_voip::CallbackObject * oobj, const Param & p )
{
    auto duration = gd_->get_duration( p.filename );

    dummy_log_trace( log_id_, "handle(): PlayFileResponse: filename %s, duration %.2f sec", p.filename.c_str(), duration );

    schedule_stop_event( p.start_req_id, p.call_id, duration, false );
}

void Wrap::handle_PlayFileStopResponse( const simple_voip::CallbackObject * oobj, const Param & p )
{
    auto * resp = simple_voip::wrap::create_PlayFileStopped( p.start_req_id );

    callback_->consume( resp );
}

void Wrap::handle_RecordFileResponse( const simple_voip::CallbackObject * oobj, const Param & p )
{
    schedule_stop_event( p.start_req_id, p.call_id, p.duration, true );
}

void Wrap::handle_RecordFileStopResponse( const simple_voip::CallbackObject * oobj, const Param & p )
{
    auto * resp = simple_voip::wrap::create_RecordFileStopped( p.start_req_id );

    callback_->consume( resp );
}

void Wrap::schedule_stop_event( uint32_t req_id, uint32_t call_id, double duration, bool is_record )
{
    dummy_log_trace( log_id_, "schedule_stop_event: req_id %u, call_id %u, duration %.2f sec, is_record %u", req_id, call_id, duration, (int)is_record );

    std::string error_msg;

    scheduler::job_id_t sched_job_id;

    auto p1 = & Wrap::handle_generate_play_stop;
    auto p2 = & Wrap::handle_generate_record_stop;
    auto p = is_record ? p2 : p1;

    auto b = scheduler::create_and_insert_timeout_job(
            & sched_job_id,
            & error_msg,
            * scheduler_,
            "timer_job",
            scheduler::Duration( duration ),
            std::bind( p, this, req_id, call_id ) );

    if( b == false )
    {
        dummy_log_error( log_id_, "cannot set timer: %s", error_msg.c_str() );
    }
    else
    {
        dummy_log_debug( log_id_, "scheduled execution in: %u sec", duration );
    }
}

void Wrap::handle_generate_play_stop( uint32_t start_req_id, uint32_t call_id )
{
    dummy_log_trace( log_id_, "handle_generate_play_stop: start_req_id %u, call_id %u", start_req_id, call_id );

    auto req_id = req_id_gen_->get_next_request_id();

    dummy_log_debug( log_id_, "handle_generate_play_stop: req_id %u, start_req_id %u, call_id %u", req_id, start_req_id, call_id );

    Param p;

    p.init( type_e::PlayFileStopRequest, start_req_id, call_id, 0, "" );

    auto b = map_req_to_param_.insert( std::make_pair( req_id, p ) ).second;

    ASSERT( b );

    auto req2 = simple_voip::create_play_file_stop_request( req_id, p.call_id );

    voips_->consume( req2 );
}

void Wrap::handle_generate_record_stop( uint32_t start_req_id, uint32_t call_id )
{
    dummy_log_trace( log_id_, "handle_generate_record_stop: start_req_id %u, call_id %u", start_req_id, call_id );

    auto req_id = req_id_gen_->get_next_request_id();

    dummy_log_debug( log_id_, "handle_generate_record_stop: req_id %u, start_req_id %u, call_id %u", req_id, start_req_id, call_id );

    Param p;

    p.init( type_e::RecordFileStopRequest, start_req_id, call_id, 0, "" );

    auto b = map_req_to_param_.insert( std::make_pair( req_id, p ) ).second;

    ASSERT( b );

    auto req2 = simple_voip::create_record_file_stop_request( req_id, p.call_id );

    voips_->consume( req2 );
}


} // namespace simple_voip_wrap
