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

#include <graphene/monitor/coin_object_monitor.hpp>
#include <graphene/chain/coin_object.hpp>

namespace graphene { namespace monitor {

using namespace chain;

uint32_t coin_object_monitor::valid_price_stale_time_guard = 300;

void coin_object_monitor::monitor_object(const database& db)
{
    uint32_t now = fc::time_point::now().sec_since_epoch();
    auto& platform_quote_base_indx = db.get_index_type<coin_index>().indices().get<by_platform_quote_base>();
    for (auto itr = platform_quote_base_indx.begin(); itr != platform_quote_base_indx.end(); ++itr)
    {
        const coin_object& coin_obj = *itr;
        if (coin_obj.status != "1")
            continue;
        const coin_dynamic_data_object& dyn_data = coin_obj.dynamic_data(db);
        ilog("coin object details: ${coin_obj}, ${dyn_data}", ("coin_obj", coin_obj)("dyn_data", dyn_data));
        uint32_t time_delta = now - dyn_data.latest_valid_time;
        if (time_delta > valid_price_stale_time_guard)
        {
            wlog("Alarm! ${platform_qbase}'s valid price not updated for a long time: ${time_delta} sec. now: ${now}, latest_valid_time: ${valid_time}, latest_feed_time: ${feed_time}, latest_valid_price: ${valid_price}, alarm guard: ${guard}",
                ("time_delta", time_delta)("now", now)("platform_qbase", coin_obj.platform_quote_base)("valid_time", dyn_data.latest_valid_time)("feed_time", dyn_data.latest_feed_time)("valid_price", dyn_data.latest_valid_price)("guard", valid_price_stale_time_guard));
        }
    }
}

void coin_object_monitor::dump_stat()
{
    ilog("------------------- coin_object_monitor::dump_stat begin -------------------");
 
    ilog("------------------- coin_object_monitor::dump_stat end -------------------");
}

void coin_object_monitor::set_valid_price_stale_time_guard(uint32_t guard)
{
    valid_price_stale_time_guard=guard;
}

} }
