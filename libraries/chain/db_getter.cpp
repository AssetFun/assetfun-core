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

#include <graphene/chain/database.hpp>

#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/chain_property_object.hpp>
#include <graphene/chain/global_property_object.hpp>
#include <graphene/chain/coin_object.hpp>
#include <graphene/chain/module_cfg_object.hpp>

#include <fc/smart_ref_impl.hpp>

namespace graphene { namespace chain {

const asset_object& database::get_core_asset() const
{
   return get(asset_id_type());
}

const global_property_object& database::get_global_properties()const
{
   return get( global_property_id_type() );
}

const chain_property_object& database::get_chain_properties()const
{
   return get( chain_property_id_type() );
}

const dynamic_global_property_object&database::get_dynamic_global_properties() const
{
   return get( dynamic_global_property_id_type() );
}

const fee_schedule&  database::current_fee_schedule()const
{
   return get_global_properties().parameters.current_fees;
}

time_point_sec database::head_block_time()const
{
   return get( dynamic_global_property_id_type() ).time;
}

uint32_t database::head_block_num()const
{
   return get( dynamic_global_property_id_type() ).head_block_number;
}

block_id_type database::head_block_id()const
{
   return get( dynamic_global_property_id_type() ).head_block_id;
}

decltype( chain_parameters::block_interval ) database::block_interval( )const
{
   return get_global_properties().parameters.block_interval;
}

const chain_id_type& database::get_chain_id( )const
{
   return get_chain_properties().chain_id;
}

const node_property_object& database::get_node_properties()const
{
   return _node_property_object;
}

node_property_object& database::node_properties()
{
   return _node_property_object;
}

uint32_t database::last_non_undoable_block_num() const
{
   return head_block_num() - _undo_db.size();
}

coin_price database::get_coin_price(const string& platform_id, const string& quote_base, uint32_t time_second)const
{
	ilog("get_coin_price platform_id: ${platform_id}, quote_base: ${quote_base}, time in second: ${time_second}",
			("platform_id", platform_id)("quote_base", quote_base)("time_second", time_second));
   string platform_quote_base = platform_id + ":" + quote_base; //e.g "1000001:BTC/USD"

   const auto& trade_pairs = get_index_type<coin_index>().indices().get<by_platform_quote_base>();
   auto itr = trade_pairs.find(platform_quote_base);
   if (itr == trade_pairs.end())
   {
	   wlog("platform_id=${platform_id}, quote_base=${quote_base} not supported", ("platform_id", platform_id)("quote_base", quote_base));
	   return coin_price();
   }
   const coin_object& coin = *itr;
   //TODO 根据时间比较，看是从fixed_data还是从dynamic_data读取价格。目前简单做，先从dynamic_data取，后面优化
   coin_dynamic_data_id_type dyn_id = coin.dynamic_coin_data_id;
   auto it_dyn = find(dyn_id);
   FC_ASSERT(it_dyn, "coin dynamic data not found for ${platform_id}:${quote_base}, dyn_id: ${dyn_id}", ("platform_id", platform_id)("quote_base", quote_base)("dyn_id", dyn_id));
   FC_ASSERT(time_second % 60 == 0, "time in second not aligned to 60");
   const coin_dynamic_data_object& dyn_data = *it_dyn;
   return dyn_data.get_coin_price(platform_id, quote_base, time_second, *this);
}

std::pair<uint32_t, coin_price> database::get_latest_valid_price(const string& platform_id, const string& quote_base)const
{
   ilog("get_latest_valid_price platfrom: ${platform_id}, quote_base: ${quote_base}", ("platform_id", platform_id)("quote_base", quote_base));
   const auto& coins_by_platform_quote_base = get_index_type<coin_index>().indices().get<by_platform_quote_base>();

   string platform_quote_base = platform_id + ":" + quote_base;
   auto itr = coins_by_platform_quote_base.find(platform_quote_base);
   if (itr == coins_by_platform_quote_base.end())
   {
      wlog("coin ${name} not supported", ("name", platform_quote_base));
      return pair<uint32_t, coin_price>();
   }
   const coin_object& coin = *itr;
   //TODO 根据时间比较，看是从fixed_data还是从dynamic_data读取价格。目前简单做，先从dynamic_data取，后面优化
   coin_dynamic_data_id_type dyn_id = coin.dynamic_coin_data_id;
   auto it_dyn = find(dyn_id);
   FC_ASSERT(it_dyn, "coin dynamic data not found for ${name}, dyn_id: ${dyn_id}", ("name", platform_quote_base)("dyn_id", dyn_id));
   const coin_dynamic_data_object& dyn_data = *it_dyn;
   return make_pair(dyn_data.latest_valid_time, dyn_data.latest_valid_price);
}

module_cfg_object database::get_module_cfg(const string& module_name)const
{
   ilog("get_module_cfg, module: ${name}", ("name", module_name));
   const auto& cfg_objs = get_index_type<module_cfg_index>().indices().get<by_name>();
   auto itr = cfg_objs.find(module_name);
   FC_ASSERT(itr != cfg_objs.end(), "module  ${name} not found", ("name", module_name)) ;
   const module_cfg_object& cfg_obj = *itr;
   ilog("cfg obj: ${obj}", ("obj", cfg_obj));
   return cfg_obj;
}


} }
