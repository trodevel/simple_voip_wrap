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


// $Revision: 9591 $ $Date:: 2018-08-03 #$ $Author: serge $

#include "wrap.h"                       // self

#include <typeindex>                    // std::type_index
#include <typeinfo>
#include <unordered_map>

#include "utils/mutex_helper.h"         // MUTEX_SCOPE_LOCK
#include "utils/dummy_logger.h"         // dummy_log
#include "utils/assert.h"               // ASSERT
#include "scheduler/onetime_job_aux.h"  // create_and_insert_one_time_job

#include "simple_voip/object_factory.h" // create_play_file_request

#include "object_factory.h"             // create_PlayFileErrorResponse
#include "objects.h"                    // PlayFileRequest

#define MODULENAME      "Wrap"

namespace simple_voip_wrap {

Wrap::Wrap():
    log_id_( 0 ),
    voips_( nullptr ), callback_( nullptr ),
    req_id_gen_( nullptr )
{
}

Wrap::~Wrap()
{
}

bool Wrap::init(
        unsigned int                        log_id,
        simple_voip::ISimpleVoip            * voips,
        simple_voip::ISimpleVoipCallback    * callback,
        utils::IRequestIdGen                * req_id_gen,
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
    req_id_gen_ = req_id_gen;

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

    typedef Wrap Type;

    typedef void (Type::*PPMF)( const simple_voip::CallbackObject * r );

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
        (this->*it->second)( obj );
    }

    callback_->consume( obj );
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

    Param p = { type_e::PlayFileRequest, 0, req->filename };

    auto b = map_req_to_param_.insert( std::make_pair( req->req_id, p ) ).second;

    ASSERT( b );

    auto req2 = simple_voip::create_play_file_request( req->req_id, req->call_id, req->filename );

    voips_->consume( req2 );
}

void Wrap::handle_RecordFileRequest( const simple_voip::ForwardObject * rreq )
{
    auto * req = dynamic_cast< const simple_voip::wrap::RecordFileRequest *>( rreq );

    // private: no mutex lock

    Param p = { type_e::RecordFileRequest, req->duration, req->filename };

    auto b = map_req_to_param_.insert( std::make_pair( req->req_id, p ) ).second;

    ASSERT( b );

    auto req2 = simple_voip::create_record_file_request( req->req_id, req->call_id, req->filename );

    voips_->consume( req2 );
}

void Wrap::handle_error( type_e type, uint32_t req_id, uint32_t errorcode, const std::string & error_msg )
{
    switch( type )
    {
    case type_e::PlayFileRequest:
    {
        auto * resp = simple_voip::wrap::create_PlayFileErrorResponse( req_id, errorcode, error_msg );

        callback_->consume( resp );
    }
        break;

    case type_e::PlayFileStopRequest:   // silently consumed
        break;

    case type_e::RecordFileRequest:
    {
        auto * resp = simple_voip::wrap::create_RecordFileErrorResponse( req_id, errorcode, error_msg );

        callback_->consume( resp );
    }
        break;

    case type_e::RecordFileStopRequest: // silently consumed
        break;

    default:
        break;
    }
}

// ISimpleVoipCallback interface
void Wrap::handle_RejectResponse( const simple_voip::CallbackObject * oobj )
{
    auto * obj = dynamic_cast< const simple_voip::RejectResponse *>( oobj );

    auto it = map_req_to_param_.find( obj->req_id );

    if( it == map_req_to_param_.end() )
    {
        callback_->consume( obj );
        return;
    }

    auto & p = it->second;

    handle_error( p.type, obj->req_id, 0, "rejected" );

    map_req_to_param_.erase( it );

    release_message( obj );
}

void Wrap::handle_ErrorResponse( const simple_voip::CallbackObject * oobj )
{
    auto * obj = dynamic_cast< const simple_voip::ErrorResponse *>( oobj );

    auto it = map_req_to_param_.find( obj->req_id );

    if( it == map_req_to_param_.end() )
    {
        callback_->consume( obj );
        return;
    }

    auto & p = it->second;

    handle_error( p.type, obj->req_id, obj->errorcode, obj->descr );

    map_req_to_param_.erase( it );

    release_message( obj );
}

void Wrap::handle_PlayFileResponse( const simple_voip::CallbackObject * oobj )
{
    auto * obj = dynamic_cast< const simple_voip::PlayFileResponse *>( oobj );

    auto it = map_req_to_param_.find( obj->req_id );

    if( it == map_req_to_param_.end() )
    {
        callback_->consume( obj );
        return;
    }

    auto & p = it->second;

    auto duration = get_duration( p.filename );

    std::string error_msg;

    scheduler::job_id_t sched_job_id;

    auto b = scheduler::create_and_insert_one_time_job(
            & sched_job_id,
            & error_msg,
            * scheduler_,
            "timer_job",
            exec_time,
            std::bind( &Wrap::handle_generate_play_stop, this, 0 ) );

    if( b == false )
    {
        dummy_log_error( log_id_, "cannot set timer: %s", error_msg.c_str() );
    }
    else
    {
        dummy_log_debug( log_id_, "scheduled execution at: %u", exec_time );
    }
}

} // namespace simple_voip_wrap
