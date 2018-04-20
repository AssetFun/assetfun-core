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
#include <graphene/chain/protocol/coin_ops.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <graphene/db/flat_index.hpp>
#include <graphene/db/generic_index.hpp>
#include <graphene/chain/module_configurator.hpp>

/**
 * @defgroup prediction_market Prediction Market
 *
 * A prediction market is a specialized BitAsset such that total debt and total collateral are always equal amounts
 * (although asset IDs differ). No margin calls or force settlements may be performed on a prediction market asset. A
 * prediction market is globally settled by the issuer after the event being predicted resolves, thus a prediction
 * market must always have the @ref global_settle permission enabled. The maximum price for global settlement or short
 * sale of a prediction market asset is 1-to-1.
 */

namespace graphene { namespace chain {
    class account_object;
    class database;
    using namespace graphene::db;


    //价格数据划分间隔
    static const uint32_t PRICE_DATA_DIVIDE_INTERVAL = 24*60*60;

    struct price_data
    {
	    //<coin price, publisher list>
	    map<share_type, set<account_id_type> > price_detail;
	    share_type price = INVALID_FEED_PRICE;  //price confirmed
	    //价格单位。这里按一种base组织价格数据。为简化逻辑，同一时间点只支持一种base。
	    string platform_quote_base = "";  // "1000001:BTC/USD"
	    share_type total_feed_count = 0;
    };

    /**
    *  @brief tracks the asset information that changes frequently
    *  @ingroup object
    *  @ingroup implementation
    *
    *  Because the asset_object is very large it doesn't make sense to save an undo state
    *  for all of the parameters that never change.   This object factors out the parameters
    *  of an asset that change in almost every transaction that involves the asset.
    *
    *  This object exists as an implementation detail and its ID should never be referenced by
    *  a blockchain operation.
    */

    class coin_price_data_object : public abstract_object<coin_price_data_object>
    {
    public:
        static const uint8_t space_id = implementation_ids;
        static const uint8_t type_id  = impl_coin_price_data_type;

        //<time, price>
        map<uint32_t, price_data> price_feeding;
    };

    class coin_dynamic_data_object : public abstract_object<coin_dynamic_data_object>
    {
    public:
        static const uint8_t space_id = implementation_ids;
        static const uint8_t type_id  = impl_coin_dynamic_data_type;

        bool is_time_continuous(uint32_t time){return (time - latest_feed_time) == 60 || (time - latest_feed_time) == 0;}

        void feed_price(
            const account_id_type& publisher, 
            const string& platform_id, 
            const string& quote_base, 
            uint32_t time, 
            const share_type& price, 
            database& db,
            bool reset_price=false
            );

        coin_price get_coin_price(const string& platform_id, const string& quote_base, uint32_t time_second, const database& db)const;

        const coin_price_data_object& get_price_data_object(uint32_t price_time, database& d);

        //<time, price> 按PRICE_DATA_DIVIDE_INTERVAL组织数据
        //比如按天组织，则一天对应一个coin_price_data_object
        map<uint32_t, coin_price_data_id_type> prices;
        //最新的喂价时间(这里价格-1也视为‘有效’价格)
        uint32_t latest_feed_time;
        //最新的有效喂价和时间（不含-1）
        coin_price latest_valid_price;
        uint32_t latest_valid_time = 0;
        uint32_t invalid_price_count = 0;
    };

    class coin_fixed_data_object : public abstract_object<coin_fixed_data_object>
    {
    public:
        static const uint8_t space_id = implementation_ids;
        static const uint8_t type_id  = impl_coin_fixed_data_type;

        //<time, price>
        map<uint32_t, price_data> price_fixed;
    };

    /**
    *  @brief tracks the parameters of an asset
    *  @ingroup object
    *
    *  All assets have a globally unique symbol name that controls how they are traded and an issuer who
    *  has authority over the parameters of the asset.
    */
    class coin_object : public graphene::db::abstract_object<coin_object>
    {
    public:
        static const uint8_t space_id = protocol_ids;
        static const uint8_t type_id  = coin_object_type;


        //coin name
        //string name;
        string platform_quote_base; //e.g "1000001:BTC/USD"
        string status; //交易对状态 "1":可见可用, "2":可见不可用, "3":不可见不可用(即下架)

        //经过确认的价格，不会再变的，放这里，减小price_feeding size
        coin_fixed_data_id_type  fixed_coin_data_id;
        //喂价更新的数据
        coin_dynamic_data_id_type  dynamic_coin_data_id;

        // The currently active flags on this permission.
        uint16_t flags = 0x01;
        //喂价人列表
        set<account_id_type> feeders;


        coin_id_type get_id()const { return id; }

        void validate()const
        {
//            // UIAs may not be prediction markets, have force settlement, or global settlements
//            if( !is_market_issued() )
//            {
//               FC_ASSERT(!(options.flags & disable_force_settle || options.flags & global_settle));
//               FC_ASSERT(!(options.issuer_permissions & disable_force_settle || options.issuer_permissions & global_settle));
//            }
        }

        template<class DB>
        const coin_dynamic_data_object& dynamic_data(const DB& db)const
        { return db.get(dynamic_coin_data_id); }

        template<class DB>
        const coin_fixed_data_object& fixed_data(const DB& db)const
        { return db.get(fixed_coin_data_id); }


    };

    typedef multi_index_container<
       coin_price_data_object,
        indexed_by<
            ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >
        >
    > coin_price_data_object_multi_index_type;
    typedef generic_index<coin_price_data_object, coin_price_data_object_multi_index_type> coin_price_data_index;

    typedef multi_index_container<
	   coin_dynamic_data_object,
        indexed_by<
            ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >
        >
    > coin_dynamic_data_object_multi_index_type;
    typedef generic_index<coin_dynamic_data_object, coin_dynamic_data_object_multi_index_type> coin_dynamic_data_index;

    typedef multi_index_container<
	   coin_fixed_data_object,
        indexed_by<
            ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >
        >
    > coin_fixed_data_object_multi_index_type;
    typedef generic_index<coin_fixed_data_object, coin_fixed_data_object_multi_index_type> coin_fixed_data_index;

    struct by_platform_quote_base;
    typedef multi_index_container<
        coin_object,
        indexed_by<
            ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >,
		  ordered_unique< tag<by_platform_quote_base>, member<coin_object, string, &coin_object::platform_quote_base> >
        >
    > coin_object_multi_index_type;
    typedef generic_index<coin_object, coin_object_multi_index_type> coin_index;

    class coin_object_creator
    {
    public:
	   static void on_coin_configed(database& db, const vector<coin_configurator::one_platform>& platforms);
    };

} } // graphene::chain

FC_REFLECT( graphene::chain::price_data,
            (price_detail)(price)(platform_quote_base)(total_feed_count)
		  )

FC_REFLECT_DERIVED( graphene::chain::coin_price_data_object, (graphene::db::object),
                    (price_feeding) )

FC_REFLECT_DERIVED( graphene::chain::coin_dynamic_data_object, (graphene::db::object),
                    (prices)(latest_feed_time)(latest_valid_price)(latest_valid_time)(invalid_price_count) )

FC_REFLECT_DERIVED( graphene::chain::coin_fixed_data_object, (graphene::db::object),
					(price_fixed) )


FC_REFLECT_DERIVED( graphene::chain::coin_object, (graphene::db::object),
                    (platform_quote_base)
                    (status)
                    (fixed_coin_data_id)
                    (dynamic_coin_data_id)
                    (flags)
                    (feeders)
                  )
