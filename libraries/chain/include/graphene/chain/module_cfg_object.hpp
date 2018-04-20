/*
 * Copyright (c) 2017 AssetFun, Inc., and contributors.
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

#include <graphene/chain/protocol/types.hpp>
#include <graphene/db/generic_index.hpp>
#include <graphene/db/object.hpp>
#include <fc/variant_object.hpp>

#include <boost/multi_index/composite_key.hpp>

namespace graphene { namespace chain {

using namespace graphene::db;

/**
 *  @brief an offer to sell a amount of a asset at a specified exchange rate by a certain time
 *  @ingroup object
 *  @ingroup protocol
 *  @ingroup market
 *
 *  This limit_order_objects are indexed by @ref expiration and is automatically deleted on the first block after expiration.
 */
class module_cfg_object : public abstract_object<module_cfg_object>
{
   public:
      static const uint8_t space_id = protocol_ids;
      static const uint8_t type_id  = module_cfg_object_type;

      module_cfg_object(){};
      module_cfg_object(const module_cfg_object& cfg_obj);
      module_cfg_object& operator=(const module_cfg_object& cfg_obj);

      virtual ~module_cfg_object(){};

      string module_name;  //模块名字，如coin，subject
      fc::variant_object module_cfg;  //模块配置
      fc::variant_object pending_module_cfg;  //TODO 配置缓冲
      time_point_sec last_update_time;  //最近一次更新时间
      account_id_type last_modifier;  //最近一次更新的人
};

struct by_id;
struct by_name;
typedef multi_index_container<
	module_cfg_object,
   indexed_by<
      ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >,
	  ordered_unique< tag<by_name>, member<module_cfg_object, string, &module_cfg_object::module_name> >
   >
> module_cfg_multi_index_type;

typedef generic_index<module_cfg_object, module_cfg_multi_index_type> module_cfg_index;


} } // graphene::chain

FC_REFLECT_DERIVED( graphene::chain::module_cfg_object,
                    (graphene::db::object),
                    (module_name)(module_cfg)(pending_module_cfg)(last_update_time)(last_modifier)
                  )
