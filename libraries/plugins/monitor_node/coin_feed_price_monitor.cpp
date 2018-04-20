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

#include <graphene/monitor/coin_feed_price_monitor.hpp>
#include <graphene/chain/witness_object.hpp>
namespace graphene { namespace monitor {

uint32_t coin_feed_price_monitor::feed_price_delay_time_guard = 300;
uint32_t coin_feed_price_monitor::no_feed_price_time_guard = 300;
uint32_t coin_feed_price_monitor::invalid_price_times_guard = 3;
const database * coin_feed_price_monitor::_db=NULL;

void coin_feed_price_monitor::do_monitor( const coin_feed_price_operation& o )
{
    ilog("coin_feed_price_monitor::do_monitor");
    if (start_new_period())
    {
        ilog("new period started. clear latest stat");
        m_platformStatLatest.clear();
        m_feederStatLatest.clear();
    }
	uint32_t now = fc::time_point::now().sec_since_epoch();
    for (const auto& price: o.prices)
    {
        m_platformStat[o.platform_id][o.quote_base].total_feed_count++;
        m_feederStat[o.publisher].total_feed_count++;

        //latest 5 minutes stat
        m_platformStatLatest[o.platform_id][o.quote_base].total_feed_count++;
		m_platformStatLatest[o.platform_id][o.quote_base].latest_feed_timestamp = now;
		accumulate accu;
		accu.feeder_id       = o.publisher;
		accu.price           = price.second;
		accu.price_timestamp = price.first;
		accu.feed_timestamp  = now;
		m_platformStatLatest[o.platform_id][o.quote_base].record.push_back(accu);

		//查询最新的有效喂价的时间戳，然后删除列表里的非必要记录的时间数据
		std::pair<uint32_t, coin_price> latest_valid_price = _db->get_latest_valid_price(o.platform_id, o.quote_base);
		std::list<accumulate>::iterator itList = m_platformStatLatest[o.platform_id][o.quote_base].record.begin();
		for( ; itList != m_platformStatLatest[o.platform_id][o.quote_base].record.end(); )
		{
			if( itList->price_timestamp <= latest_valid_price.first )
			{
				itList = m_platformStatLatest[o.platform_id][o.quote_base].record.erase( itList);
			}
			else
				itList++;
		}

        m_feederStatLatest[o.publisher].total_feed_count++;
		m_feederStatLatest[o.publisher].latest_feed_timestamp = now;
		m_feederStatLatest[o.publisher].platform_quote_base_feed_times[o.platform_id][o.quote_base]++;//
        if (price.second == INVALID_FEED_PRICE)
        {
            m_platformStat[o.platform_id][o.quote_base].invalid_feed_count++;
            m_feederStat[o.publisher].invalid_feed_count++;

            m_platformStatLatest[o.platform_id][o.quote_base].invalid_feed_count++;
            m_feederStatLatest[o.publisher].invalid_feed_count++;

			elog("Witness[${witness_id}] feed ${platform_id} ${quote_base} invalid price.",("witness_id", o.publisher)("platform_id", o.platform_id)("quote_base", o.quote_base));
        }

		//o.publisher is eg. 1.2.6

		//[lilianwen add 20181-20]增加喂价延时监控
		uint32_t time_diff = now - price.first;
		if (time_diff > feed_price_delay_time_guard)
		{
			elog("Current feed price[${platform_id}:${quote_base}] is ${time_diff} ago.", ("platform_id", o.platform_id)("quote_base",o.quote_base)("time_diff",time_diff));
		}		
    }
	//[lilianwen add 2018-1-20]检测所有活跃见证人是否喂价正常
	const global_property_object &po = _db->get_global_properties();

	auto ite_witness_id=po.active_witnesses.begin();
	for (;ite_witness_id != po.active_witnesses.end(); ite_witness_id++ )
	{
		//elog("active_witnessid:${ite_witness_id}",("ite_witness_id", *ite_witness_id));
		const auto& idx = _db->get_index_type<witness_index>().indices().get<by_id>();
		object_id_type id = *ite_witness_id;
		
		auto itr = idx.find(id);
		if( itr != idx.end() )
		{
			const witness_object &wo = *itr;
			uint32_t diff=now - m_feederStatLatest[wo.witness_account].latest_feed_timestamp;
			if(diff > no_feed_price_time_guard)//距离上次喂价已经有这么久没有喂价了
			{
				//elog("now=${now}, latest_feed_timestamp=${latest_feed_timestamp}", ("now", now)("latest_feed_timestamp", m_feederStat[id].latest_feed_timestamp));
				elog("Witness[${witness_id}]'s feeding price procedure may be dumped(${diff} > ${no_feed_price_time_guard}).", ("witness_id", id)("diff", diff)("no_feed_price_time_guard",no_feed_price_time_guard));
			}
			else
			{
				ilog("Witness[${witness_id}]'s feeding price procedure is online.", ("witness_id", id));
			}
		}
	}
}

void coin_feed_price_monitor::dump_stat()
{
    ilog("------------------- coin_feed_price_monitor::dump_stat begin -------------------");
    for(const auto& platform: m_platformStat)
    {
        auto& platform_id = platform.first;
        for(const auto& qbase : platform.second)
        {
            auto& quote_base = qbase.first;
            auto& total_count = qbase.second.total_feed_count;
            auto& invalid_count = qbase.second.invalid_feed_count;
            auto valid_count = total_count - invalid_count;
            auto valid_rate = valid_count*100/total_count;
            ilog("Accumulated stat. platform_id: ${platform_id}, quote_base: ${qbase}, total_feed_count: ${total_count}, valid_feed_count: ${valid_count}, invalid_feed_count: ${invalid_count}, valid_rate: ${valid_rate}",
                ("platform_id", platform_id)("qbase", quote_base)("total_count", total_count)("valid_count", valid_count)("invalid_count", invalid_count)("valid_rate", valid_rate));
        }
    }

    for(const auto& platform: m_platformStatLatest)
    {
        auto& platform_id = platform.first;
        for(const auto& qbase : platform.second)
        {
            auto& quote_base = qbase.first;
            auto& total_count = qbase.second.total_feed_count;
            auto& invalid_count = qbase.second.invalid_feed_count;
            auto valid_count = total_count - invalid_count;
            auto valid_rate = valid_count*100/total_count;
            ilog("Latest 5 minutes stat. platform_id: ${platform_id}, quote_base: ${qbase}, total_feed_count: ${total_count}, valid_feed_count: ${valid_count}, invalid_feed_count: ${invalid_count}, valid_rate: ${valid_rate}",
                ("platform_id", platform_id)("qbase", quote_base)("total_count", total_count)("valid_count", valid_count)("invalid_count", invalid_count)("valid_rate", valid_rate));
			
			elog("[${platform}][${quote_base}] feed price situation as blow(size:${size}):", 
				("platform", platform_id)("quote_base", quote_base)("size", m_platformStatLatest[platform_id][quote_base].record.size()));
			//std::list<accumulate>::iterator itList = qbase.second.record.begin();
			//for( ; itList != qbase.second.record.end();  itList++)
			std::list<accumulate>::iterator itList = m_platformStatLatest[platform_id][quote_base].record.begin();
			for( ; itList != m_platformStatLatest[platform_id][quote_base].record.end();  itList++)
			{
				elog("feeder_id[${feeder_id}]price_timestamp[${price_timestamp}]feed_timestamp[${feed_timestamp}]price[${price}]",
					("feeder_id", itList->feeder_id)("price_timestamp", itList->price_timestamp)
					("feed_timestamp", itList->feed_timestamp)("price", itList->price));					
			}
		
		}
    }

    for(const auto& feeder: m_feederStat)
    {
        auto& feeder_id = feeder.first;
        auto& total_count = feeder.second.total_feed_count;
        auto& invalid_count = feeder.second.invalid_feed_count;
        auto valid_count = total_count - invalid_count;
        auto valid_rate = valid_count*100/total_count;
        ilog("Accumulated stat. feeder: ${feeder_id}, total_feed_count: ${total_count}, valid_count: ${valid_count}, invalid_feed_count: ${invalid_count}, valid_rate: ${valid_rate}",
            ("feeder_id", feeder_id)("total_count", total_count)("valid_count", valid_count)("invalid_count", invalid_count)("valid_rate", valid_rate));
    }

    for(const auto& feeder: m_feederStatLatest)
    {
        auto& feeder_id = feeder.first;
        auto& total_count = feeder.second.total_feed_count;
        auto& invalid_count = feeder.second.invalid_feed_count;
        auto valid_count = total_count - invalid_count;
		if(total_count != 0)
		{
			auto valid_rate = valid_count*100/total_count;
			ilog("Latest 5 minutes stat. feeder: ${feeder_id}, total_feed_count: ${total_count}, valid_count: ${valid_count}, invalid_feed_count: ${invalid_count}, valid_rate: ${valid_rate}",
				("feeder_id", feeder_id)("total_count", total_count)("valid_count", valid_count)("invalid_count", invalid_count)("valid_rate", valid_rate));
		}
		else
		{
			elog("feeder[${feeder_id}]'s total count is zero.", ("feeder_id", feeder_id));
		}
		if(invalid_count >=invalid_price_times_guard)
		{
			elog("feeder[${feeder_id}] feed ${invalid_count} times invalid price in latest 5 minutes.", ("feeder_id",feeder_id)("invalid_count",invalid_count));
		}

		//打印每个交易对的喂价次数
		for(const auto& feeder: m_feederStatLatest)
		{
			for(const auto& platform: feeder.second.platform_quote_base_feed_times)
			{
				for (const auto& quote_base:platform.second)
				{
					ilog("feeder[${feeder_id}][${platform}][${quote_base}]=${times}", ("feeder_id", feeder.first)("platform", platform.first)("quote_base", quote_base.first)
						("times", quote_base.second));
				}
			}
		}
	}

	
	
    ilog("------------------- coin_feed_price_monitor::dump_stat end -------------------");
}

void coin_feed_price_monitor::set_feed_price_delay_time_guard(uint32_t guard)
{
	feed_price_delay_time_guard=guard;
}

void coin_feed_price_monitor::set_no_feed_price_time_guard(uint32_t guard)
{
	no_feed_price_time_guard=guard;
}

void coin_feed_price_monitor::set_invalid_price_times_guard(uint32_t guard)
{
	invalid_price_times_guard=guard;
}

void coin_feed_price_monitor::set_database(const database *db)
{
	_db=db;
}

} }
