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

#include <graphene/chain/protocol/operations.hpp>
#include <graphene/monitor/op_monitor.hpp>
#include <graphene/chain/protocol/coin_ops.hpp>
#include <graphene/chain/database.hpp>

namespace graphene { namespace monitor {

    using namespace chain;

	struct accumulate{
		account_id_type feeder_id;
		share_type price;
		uint32_t price_timestamp;
		uint32_t feed_timestamp;
	};

    struct feed_stat
    {
        uint64_t total_feed_count;
        uint64_t invalid_feed_count;
		uint32_t latest_feed_timestamp;//最近的一次喂价时刻时间戳
		std::list<accumulate> record;//喂价的累积统计

		std::map<string, std::map<string, uint32_t> > platform_quote_base_feed_times;
    };

    class coin_feed_price_monitor : public op_monitor<coin_feed_price_monitor>
    {
        public:

            typedef coin_feed_price_operation operation_type;

            coin_feed_price_monitor(){};
            virtual ~coin_feed_price_monitor(){};
            

            void do_monitor( const coin_feed_price_operation& o );

            void dump_stat() override;

			static void set_feed_price_delay_time_guard(uint32_t guard);
			static void set_no_feed_price_time_guard(uint32_t guard);
			static void set_invalid_price_times_guard(uint32_t guard);
			static void set_database(const database *db);

		

        private:
            //<platform <quote_base, feed_stat> >
            std::map<string, std::map<string, feed_stat> > m_platformStat;
            //<feeder, feed_stat>
            std::map<account_id_type, feed_stat> m_feederStat;
            //<platform <quote_base, feed_stat> >, latest 5 minitus stat
            std::map<string, std::map<string, feed_stat> > m_platformStatLatest;
            //<feeder, feed_stat>, latest 5 minitus stat
            std::map<account_id_type, feed_stat> m_feederStatLatest;

			static uint32_t feed_price_delay_time_guard;
			static uint32_t no_feed_price_time_guard;
			static uint32_t invalid_price_times_guard;
			static const database *_db;

    };

} } 
