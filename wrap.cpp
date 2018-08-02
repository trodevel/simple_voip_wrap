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

#include "wrap.h"                       // self

#include <typeindex>                    // std::type_index
#include <typeinfo>
#include <unordered_map>

#include "utils/mutex_helper.h"         // MUTEX_SCOPE_LOCK
#include "utils/dummy_logger.h"         // dummy_log
#include "utils/assert.h"               // ASSERT

#define MODULENAME      "Wrap"

namespace simple_voip_wrap {

Wrap::Wrap():
    log_id_( 0 ),
    voips_( nullptr ), callback_( nullptr )
{
}

Wrap::~Wrap()
{
}

bool Wrap::init(
        unsigned int                        log_id,
        simple_voip::ISimpleVoip            * voips,
        simple_voip::ISimpleVoipCallback    * callback,
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

    return true;
}

void Wrap::consume( const simple_voip::ForwardObject* obj )
{
    MUTEX_SCOPE_LOCK( mutex_ );

    typedef Wrap Type;

    typedef void (Type::*PPMF)( const simple_voip::ForwardObject * r );

#define HANDLER_MAP_ENTRY(_v)       { typeid( simple_voip::_v ),            & Type::handle_##_v }

    static const std::unordered_map<std::type_index, PPMF> funcs =
    {
        HANDLER_MAP_ENTRY( InitiateCallRequest ),
        HANDLER_MAP_ENTRY( DropRequest ),
    };

#undef HANDLER_MAP_ENTRY

    auto it = funcs.find( typeid( * obj ) );

    if( it != funcs.end() )
    {
        (this->*it->second)( obj );
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
        HANDLER_MAP_ENTRY( InitiateCallResponse ),
        HANDLER_MAP_ENTRY( ErrorResponse ),
        HANDLER_MAP_ENTRY( RejectResponse ),
        HANDLER_MAP_ENTRY( DropResponse ),
        HANDLER_MAP_ENTRY( ConnectionLost ),
        HANDLER_MAP_ENTRY( Failed ),
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

void Wrap::handle_InitiateCallRequest( const simple_voip::ForwardObject * rreq )
{
    auto * req = dynamic_cast< const simple_voip::InitiateCallRequest *>( rreq );

    // private: no mutex lock

    log_stat();

    if( get_num_of_activities() >= cfg_.max_active_calls )
    {
        request_queue_.push_back( req );

        dummy_log_debug( log_id_, "insert_job: inserted job %u", req->req_id );

        log_stat();

        return;
    }

    process( req );
}

void Wrap::handle_DropRequest( const simple_voip::ForwardObject * rreq )
{
    auto * req = dynamic_cast< const simple_voip::DropRequest *>( rreq );

    if( active_call_ids_.count( req->call_id ) == 0 )
    {
        dummy_log_warn( log_id_, "unknown call id %u", req->call_id );

        voips_->consume( req );

        return;
    }

    auto _b = map_drop_req_id_to_call_id_.insert( std::make_pair( req->req_id, req->call_id ) ).second;

    ASSERT( _b );

    voips_->consume( req );
}

// ISimpleVoipCallback interface
void Wrap::handle_InitiateCallResponse( const simple_voip::CallbackObject * oobj )
{
    auto * obj = dynamic_cast< const simple_voip::InitiateCallResponse *>( oobj );

    dummy_log_trace( log_id_, "initiated call: %u", obj->call_id );

    auto it = active_request_ids_.find( obj->req_id );

    if( it == active_request_ids_.end() )
    {
        dummy_log_error( log_id_, "unknown call id %u", obj->call_id );
        return;
    }

    active_request_ids_.erase( it );

    auto b = active_call_ids_.insert( obj->call_id ).second;

    if( b == false )
    {
        dummy_log_error( log_id_, "cannot insert call id %u - already exists", obj->call_id );

        ASSERT( 0 );

        return;
    }

    dummy_log_debug( log_id_, "call id %u - active", obj->call_id );

    log_stat();
}

void Wrap::handle_RejectResponse( const simple_voip::CallbackObject * oobj )
{
    auto * obj = dynamic_cast< const simple_voip::RejectResponse *>( oobj );

    auto it = active_request_ids_.find( obj->req_id );

    if( it == active_request_ids_.end() )
    {
        erase_failed_drop_request( obj->req_id );
        return;
    }

    active_request_ids_.erase( it );

    process_jobs();
}

void Wrap::handle_ErrorResponse( const simple_voip::CallbackObject * oobj )
{
    auto * obj = dynamic_cast< const simple_voip::ErrorResponse *>( oobj );

    auto it = active_request_ids_.find( obj->req_id );

    if( it == active_request_ids_.end() )
    {
        erase_failed_drop_request( obj->req_id );
        return;
    }

    active_request_ids_.erase( it );

    process_jobs();
}

} // namespace simple_voip_wrap
