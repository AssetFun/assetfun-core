/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#pragma once

#include <graphene/app/plugin.hpp>
#include <graphene/chain/database.hpp>

#include <graphene/monitor/op_monitor.hpp>
#include <graphene/monitor/object_monitor.hpp>

#include <fc/thread/future.hpp>

namespace graphene { namespace monitor {

    using namespace chain;


    class monitor_plugin : public graphene::app::plugin
    {
        public:
            monitor_plugin();
            virtual ~monitor_plugin();

            std::string plugin_name()const override;
            virtual void plugin_set_program_options(
            boost::program_options::options_description& cli,
            boost::program_options::options_description& cfg) override;
            virtual void plugin_initialize(const boost::program_options::variables_map& options) override;
            virtual void plugin_startup() override;

            template<typename OpMonitorType>
            void register_op_monitor()
            {
                uint32_t op_id = operation::tag<typename OpMonitorType::operation_type>::value;
                assert(_operation_monitors.find(op_id) == _operation_monitors.end());
                _operation_monitors[op_id].reset( new operation_monitor_imp<OpMonitorType>() );
            }

            template<typename ObjMonitorType>
            void register_object_monitor()
            {
                uint16_t space_type = ObjMonitorType::object_type::space_id;
                space_type = space_type << 8 | ObjMonitorType::object_type::type_id;
                assert(_object_monitors.find(space_type) == _object_monitors.end());
                _object_monitors[space_type].reset( new ObjMonitorType() );
            }

        private:

            void schedule_task_loop();

            void monitor_check_loop();

            void monitor_block( const signed_block& b );
            void monitor_operation(const operation& op);
            void monitor_objects();

            void dump_statistics();

            uint32_t invalid_price_count = 0;
            fc::future<void> _monitor_check_task;

            map< uint32_t, unique_ptr<operation_monitor> >     _operation_monitors;
            map<uint64_t, uint32_t> block_trans_count;

            //<space_type, object_monitor>
            map<uint16_t, unique_ptr<object_monitor>> _object_monitors;
    };

} } 
