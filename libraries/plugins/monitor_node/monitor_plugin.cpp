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

#include <graphene/monitor/monitor_plugin.hpp>
#include <graphene/monitor/op_monitor.hpp>
#include <graphene/monitor/coin_feed_price_monitor.hpp>
#include <graphene/monitor/coin_object_monitor.hpp>
#include <graphene/monitor/witness_monitor.hpp>

#include <graphene/chain/config.hpp>
#include <graphene/chain/database.hpp>

#include <fc/smart_ref_impl.hpp>
#include <fc/thread/thread.hpp>

#include <iostream>

namespace graphene { namespace monitor {

monitor_plugin::monitor_plugin()
{
}

monitor_plugin::~monitor_plugin()
{
}

std::string monitor_plugin::plugin_name()const
{
    return "monitor_node";
}

void monitor_plugin::plugin_set_program_options(
    boost::program_options::options_description& cli,
    boost::program_options::options_description& cfg
    )
{
    cli.add_options()
          ("price-stale-alarm-guard", boost::program_options::value<uint32_t>(), "raise an alarm if price not updated for a long time exceeds the setting")
          ;
	cli.add_options()
		("feed-price-delay-time-guard", boost::program_options::value<uint32_t>(), "raise an alarm if feed price delay exceeds the setting")
		;
	cli.add_options()
		("no-feed-price-time-guard", boost::program_options::value<uint32_t>(), "raise an alarm if no feed price time exceeds the setting")
		;
	cli.add_options()
		("invalid-price-times-guard", boost::program_options::value<uint32_t>(), "raise an alarm if witness feed invalid price times exceeds the setting")
		;
	cli.add_options()
		("witness-online-alarm-guard", boost::program_options::value<uint32_t>(), "raise an alarm if witness do not generate block time exceeds the setting")
		;
    cfg.add(cli);
}

void monitor_plugin::plugin_initialize(const boost::program_options::variables_map& options)
{
    ilog("monitor_plugin initialization");
    if( options.count("price-stale-alarm-guard") )
    {
        //
        uint32_t guard = options["price-stale-alarm-guard"].as<uint32_t>();
        coin_object_monitor::set_valid_price_stale_time_guard(guard);
		ilog("price-stale-alarm-guard: ${guard}", ("guard", guard));
		std::cout << "price-stale-alarm-guard: " << guard << std::endl; 
    }
    else
    {
        ilog("price-stale-alarm-guard not set. Use default value.");
        std::cout << "price-stale-alarm-guard not set. Use default value." << std::endl;
        coin_object_monitor::set_valid_price_stale_time_guard(600);
    }

	if( options.count("feed-price-delay-time-guard") )
	{
		uint32_t guard = options["feed-price-delay-time-guard"].as<uint32_t>();
		coin_feed_price_monitor::set_feed_price_delay_time_guard(guard);
		ilog("feed-price-delay-time-guard: ${guard}", ("guard", guard));
		std::cout << "feed-price-delay-time-guard: " << guard << std::endl; 
	}
	else
	{
		ilog("feed-price-delay-time-guard not set. Use default value.");
		std::cout << "feed-price-delay-time-guard not set. Use default value." << std::endl;
		coin_feed_price_monitor::set_feed_price_delay_time_guard(600);
	}

	if( options.count("no-feed-price-time-guard") )
	{
		uint32_t guard = options["no-feed-price-time-guard"].as<uint32_t>();
		coin_feed_price_monitor::set_no_feed_price_time_guard(guard);
		ilog("no-feed-price-time-guard: ${guard}", ("guard", guard));
		std::cout << "no-feed-price-time-guard: " << guard << std::endl; 
	}
	else
	{
		ilog("no-feed-price-time-guard not set. Use default value.");
		std::cout << "no-feed-price-time-guard not set. Use default value." << std::endl;
		coin_feed_price_monitor::set_no_feed_price_time_guard(600);
	}

	if( options.count("invalid-price-times-guard") )
	{
		uint32_t guard = options["invalid-price-times-guard"].as<uint32_t>();
		coin_feed_price_monitor::set_invalid_price_times_guard(guard);
		ilog("invalid-price-times-guard: ${guard}", ("guard", guard));
		std::cout << "invalid-price-times-guard: " << guard << std::endl; 
	}
	else
	{
		ilog("invalid-price-times-guard not set. Use default value.");
		std::cout << "invalid-price-times-guard not set. Use default value." << std::endl;
		coin_feed_price_monitor::set_invalid_price_times_guard(600);
	}

	if( options.count("witness-online-alarm-guard") )
	{
		uint32_t guard = options["witness-online-alarm-guard"].as<uint32_t>();
		witness_monitor::set_witness_active_time_guard(guard);
		ilog("witness-online-alarm-guard: ${guard}", ("guard", guard));
		std::cout << "witness-online-alarm-guard: " << guard << std::endl;

	}
	else
	{
		ilog("witness-online-alarm-guard not set. Use default value.");
		std::cout << "witness-online-alarm-guard not set. Use default value." << std::endl;
		witness_monitor::set_witness_active_time_guard(600);
	}


	

    database().applied_block.connect( [&]( const signed_block& b){ monitor_block(b); } );

	//const database &db=database();
	coin_feed_price_monitor::set_database(&(database()));

    register_op_monitor<coin_feed_price_monitor>();

    //object monitor
    register_object_monitor<coin_object_monitor>();
	register_object_monitor<witness_monitor>();
}

void monitor_plugin::plugin_startup()
{
    ilog("monitor_plugin startup");
    schedule_task_loop();
}


void monitor_plugin::monitor_block( const signed_block& b )
{
    auto head = database().head_block_num();
    ilog("monitor block: head block num: ${head}, new block: ${block}", ("head", head)("block", b));
    //统计每个块的交易数
    block_trans_count[head] = b.transactions.size();
    ilog("block num: ${block_num}, trans count: ${trans_count}", ("block_num", head)("trans_count",b.transactions.size()));

    for( const auto& trx : b.transactions )
    {
        //统计每个交易的操作数

        for(const auto& op : trx.operations)
        {
            //调用操作对应的监控统计函数
            monitor_operation(op);

        }
    }
}

void monitor_plugin::monitor_operation(const operation& op)
{
    int i_which = op.which();
    uint32_t u_which = uint32_t( i_which );
    if( i_which < 0 )
        assert( "Negative operation tag" && false );
    if (_operation_monitors.find(u_which) != _operation_monitors.end())
    {
        unique_ptr<operation_monitor>& monitor = _operation_monitors[u_which];
        if (!monitor)
            assert( "No registered monitor for this operation" && false );
        monitor->monitor_op(op);
    }
}

void monitor_plugin::monitor_objects()
{
    for(auto it = _object_monitors.begin(); it != _object_monitors.end(); ++it)
    {
        it->second->monitor_object(database());
    }
}

void monitor_plugin::schedule_task_loop()
{
    ilog("schedule_task_loop");
    //Schedule for the next second's tick regardless of chain state
    // If we would wait less than 50ms, wait for the whole second.
    fc::time_point now = fc::time_point::now();
    uint32_t period = 5000000;
    int64_t time_to_next_period = period - (now.time_since_epoch().count() % period);
    if( time_to_next_period < 50000 )      // we must sleep for at least 50ms
        time_to_next_period += period;

    fc::time_point next_wakeup( now + fc::microseconds( time_to_next_period ) );

    //wdump( (now.time_since_epoch().count())(next_wakeup.time_since_epoch().count()) );
    _monitor_check_task = fc::schedule([this]{monitor_check_loop();},
                                         next_wakeup, "monitor checking");
}

void monitor_plugin::monitor_check_loop()
{
    ilog("monitor checking...");

    //check latest block time

    //统计交易数

    //check objects
    monitor_objects();

    //dump statistics
    dump_statistics();

    schedule_task_loop();
}

void monitor_plugin::dump_statistics()
{
    //just a test
    //ilog("block_trans_count: ${trans_count}", ("trans_count", block_trans_count));
    for (const auto& operation_monitor: _operation_monitors)
    {
        //
        if (operation_monitor.second)
        {
            operation_monitor.second->dump_stat();
        } 
    }

    for (const auto& object_monitor: _object_monitors)
    {
        if (object_monitor.second)
        {
            object_monitor.second->dump_stat();
        } 
    }
}

} }
